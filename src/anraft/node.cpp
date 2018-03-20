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

// Author: chenbang@antalk.com

#include "bthread/bthread.h"
#include "node.h"
#include "raft.h"


namespace anraft {

Node& Node::StartNode(const Config& config, const std::vector<Peer> &peers) {
    Raft& raft = Raft::NewRaft(config);
    // become the follower at term 1 and apply initial configuration
    // entries of term 1
    raft.BecomeFollower(1, 0);

    for (auto &x : peers) {
        ConfChange conf_change;
        conf_change.set_type(ConfChangeAddNode);
        conf_change.set_nodeid(x.id);
        conf_change.set_context(x.context);

        LogEntry log_entry;
        if (!conf_change.SerializeToString(log_entry.mutable_data())) {
            //TODO panic
        }
        log_entry.set_type(EntryConfChange);
        log_entry.set_term(1);
        log_entry.set_index(RaftLog::GetRaftLog().LastIndex() + 1);
        RaftLog::GetRaftLog().Append(log_entry);
    }

    // Mark these initial entries as committed.
    // TODO(bdarnell): These entries are still unstable; do we need to preserve
    // the invariant that committed < unstable?
    RaftLog::GetRaftLog().SetCommited(RaftLog::GetRaftLog().LastIndex());

    // Now apply them, mainly so that the application can call Campaign
    // immediately after StartNode in tests. Note that these nodes will
    // be added to raft twice: here and when the application's Ready
    // loop calls ApplyConfChange. The calls to addNode must come after
    // all calls to raftLog.append so progress.next is set after these
    // bootstrapping entries (it is an error if we try to append these
    // entries since they have already been committed).
    // We do not set raftLog.applied so the application will be able
    // to observe all conf changes via Ready.CommittedEntries.
    for (auto &x : peers) {
        raft.AddNode(x.id);
    }

    static Node node;
    
    //bthread_t tid;
    //if (bthread_start_background(&tid, NULL, Node::Run, (void*)&node) != 0) {
    //    LOG(ERROR) << "Fail to create bthread: Node::Run";
    //    //return; TODO
    //}

    bthread::execution_queue_start(&node.queue_id_,
                                    NULL,
                                    Node::Run,
                                    (void*)&node);

    return node;
}

Node& Node::RestartNode(const Config& config) {

}


Node::Node() {}

Node& Node::operator=(const Node&) {}

void Node::Tick() {
    NodeRecvMsg msg;
    msg.type = MsgTypeTick;
    if (bthread::execution_queue_execute(queue_id_, msg) != 0) {
        return;
    }
}


int Node::Run(void* meta, bthread::TaskIterator<NodeRecvMsg>& iter) {
    Node* node = (Node*)meta;
    if (iter.is_queue_stopped()) {
        //node->do_shutdown(); //TODO
        return 0;
    }

    for (; iter; ++iter) {
        switch (iter->type) {
        case MsgTypeProp:
            Message* msg = (Message*)iter->body;
            msg->set_from(Raft::GetRaft().GetID());
            Raft::GetRaft().Step(msg);
            break;
        case MsgTypeReady:
            break;

        case MsgTypeTick:
            break;
        }
    }
}

} //namespace anraft



