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
#include "bthread/bthread.h"
#include "bthread/types.h"
#include "bthread/unstable.h"
#include "butil/time.h"
#include "raft_node.h"

namespace example {

RaftNode& RaftNode::NewRaftNode(int id,
                                const std::vector<std::string>& peers,
                                bool join,
                                GetSnapshotFunc_t getsnapshot_func) {
    static RaftNode g_raft_node(id, peers, join, getsnapshot_func);
    return g_raft_node;
}

RaftNode::RaftNode(int id,
                   const std::vector<std::string>& peers,
                   bool join,
                   GetSnapshotFunc_t getsnapshot_func) 
    : raft_storage_(anraft::MemoryStorage::NewMemoryStorage()) {

}

void RaftNode::StartRaft() {
    //TODO: raftsnap initailization

    //TODO: WAL initailization

    //raft.StartNode or raft.RestartNode

}

void RaftNode::ServeChannels() {


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
}

void RaftNode::OnTickTimer(void *arg) {
    RaftNode* raftnode = (RaftNode*)arg;
    if (raftnode)
        raftnode->node_.Tick();
}

} //namespace example