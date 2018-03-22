// Copyright (c) 2015 Antalk, Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: chenbang (chenbang@antalk.com)

#include "../raftexample/raft_node.h"

#include <functional>
#include "bthread/bthread.h"
#include "bthread/types.h"
#include "bthread/unstable.h"
#include "butil/time.h"
#include "butil/file_util.h"
#include "brpc/uri.h"

namespace example {

NewRaftNodeReturnType RaftNode::NewRaftNode(int id,
                                const std::vector<std::string>& peers,
                                bool join,
                                GetSnapshotFunc_t getsnapshot_func,
                                std::promise<std::string> promise_propose,
                                std::promise<anraft::ConfChange> promise_confchange) {
    static RaftNode g_raft_node(id, peers, join, getsnapshot_func, std::move(promise_propose), std::move(promise_confchange));

    bthread_t tid;
    if (bthread_start_background(&tid, NULL, RaftNode::StartRaft, (void*)&g_raft_node) != 0) {
    	LOG(ERROR) << "Fail to create bthread: StartRaft";
    	//return; TODO
    }

    return make_tuple(std::move(g_raft_node.promise_commit_), std::move(g_raft_node.promise_error_), std::move(g_raft_node.snapshotter_ready_));
}


bool RaftNode::Init(int id,
                    const std::vector<std::string>& peers,
                    bool join,
                    GetSnapshotFunc_t getsnapshot_func) {
    id_ = id;
    peers_ = peers;
    join_ = join;
    get_snapshot_func_ = getsnapshot_func;
    return true;
}

bool RaftNode::Start() {
    bthread_t tid;
    if (bthread_start_background(&tid, NULL, RaftNode::StartRaft, this) != 0) {
    	LOG(ERROR) << "Fail to create bthread: StartRaft";
    	return false;
    }

    return true;
}

RaftNode& RaftNode::Instance() {
    static RaftNode g_raft_node;
    return g_raft_node;
}

RaftNode::RaftNode(int id,
                   const std::vector<std::string>& peers,
                   bool join,
                   GetSnapshotFunc_t getsnapshot_func,
                   std::promise<std::string> promise_propose,
                   std::promise<anraft::ConfChange> promise_confchange) 
        : id_(id),
        peers_(peers),
        join_(join),
        get_snapshot_func_(getsnapshot_func),
        promise_propose_(std::move(promise_propose)),
		promise_confchange_(std::move(promise_confchange)),
		raft_storage_(anraft::MemoryStorage::NewMemoryStorage()) {
    snap_count_ = default_snap_count;
    waldir_ = "raftexample-" + std::to_string(id_);
    snapdir_ = "raftexample-" + std::to_string(id_) + "-snap";

}

RaftNode::RaftNode() : raft_storage_(anraft::MemoryStorage::NewMemoryStorage()) {}

void* RaftNode::StartRaft(void* arg) {
    //get arg
    if (!arg) {
        LOG(ERROR) << "arg is null";
        return nullptr;
    }
    RaftNode* raft_node = (RaftNode*)arg;

    //TODO raftsnap initailization
    butil::File::Error error;
    butil::FilePath path(raft_node->snapdir_);
    butil::FilePath dir = path.DirName();
    if (!butil::CreateDirectoryAndGetError(dir, &error) && error != butil::File::FILE_ERROR_EXISTS) {
        LOG(ERROR) << "Fail to create directory=`" << dir.value()
            << "', " << error;
        return nullptr;
    }

    //TODO: WAL initailization
    bool oldwal = false;

    //raft.StartNode or raft.RestartNode
    anraft::Config config(raft_node->id_, 10, 1, 1024 * 1024, 256);
    if (oldwal) {
        //TODO
    }
    else {
        std::vector<anraft::Peer> peers;
        if (raft_node->join_) {
            for (int i = 0; i < peers.size(); i++) {
                peers.push_back(anraft::Peer(i++));
            }
        }
        raft_node->node_ = anraft::Node::StartNode(config, peers);
    }


    raft_node->ServeRaft();
    raft_node->StartServeChannels();
    return nullptr;
}

void RaftNode::StartServeChannels() {
    //start tick timer
    bthread_timer_t timer_id = 0;
    int64_t election_timeout_ = 0;
    int rc = bthread_timer_add(&timer_id,
                               butil::microseconds_to_timespec(election_timeout_),
                               OnTickTimer, this);
    if (rc) {
        LOG(ERROR) << "Fail to add timer: " << berror(rc);
        return;
    }


    bthread::execution_queue_start(&propose_queue_id_,
                                    NULL,
                                    RaftNode::ServeProposeChannels,
                                    (void*)this);

    bthread::execution_queue_start(&queue_id_,
                                    NULL,
                                    RaftNode::ServeChannels,
                                    (void*)this);
}

int RaftNode::ServeChannels(void* meta, bthread::TaskIterator<ChannalMsg>& iter) {
    RaftNode* raftnode = (RaftNode*)meta;
    if (iter.is_queue_stopped()) {
        //node->do_shutdown(); //TODO
        return 0;
    }

    for (; iter; ++iter) {
        switch (iter->type()) {
        case ChannalTypeReady:
            //����HardState, Entries���־û��洢��(wal)
            //����snapshot
            //���Entries������raftStorage��(��Ҫ���raft�ڲ���raftLog)
            //����Messages��Follower
            //��committedEntriesͨ��commitC��¶���û�����Ӧ�õ�State Machine��
            //������ǰ״̬�Ƿ���Ҫ����snapshot����
            //����raft���Ǵ�������һ��Ready
            std::vector<anraft::LogEntry> ents;
            raftnode->PublishEntries(ents);
            break;

        }
    }
}





int RaftNode::ServeProposeChannels(void* meta, bthread::TaskIterator<ChannalMsg>& iter) {
    RaftNode* raftnode = (RaftNode*)meta;
    if (iter.is_queue_stopped()) {
        //node->do_shutdown(); //TODO
        return 0;
    }

    for (; iter; ++iter) {
        switch (iter->type()) {
        case ChannalTypePropose:
            if (raftnode)
                raftnode->node_.Propose(iter->propose());
            break;

        }
    }
}


void RaftNode::Propose(const std::string& buf) {
    ChannalMsg msg;
    msg.set_type(ChannalTypePropose);
    msg.set_propose(buf);
    if (bthread::execution_queue_execute(propose_queue_id_, msg) != 0) {
        return;
    }
}

anraft::RaftError RaftNode::Process(anraft::Message msg) {}

void RaftNode::ServeRaft() {
    std::string host;
    int port, parse_error;
    if ((parse_error = brpc::ParseHostAndPortFromURL(peers_[id_ - 1].c_str(), &host, &port)) < 0) {
        LOG(ERROR) << "Fail to parse url: " << parse_error;
        return;
    }
}

void RaftNode::OnTickTimer(void *arg) {
    RaftNode* raftnode = (RaftNode*)arg;
    if (raftnode)
        raftnode->node_.Tick();
}


bool RaftNode::PublishEntries(std::vector<anraft::LogEntry>& ents) {
    for (auto &x : ents) {
        if (x.type() == anraft::EntryNormal) {
            if (x.data().empty())
                continue;


        } else {}
    }
}


} //namespace example
