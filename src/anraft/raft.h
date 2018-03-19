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

#ifndef SOURCE_DIRECTORY_RAFT_H_
#define SOURCE_DIRECTORY_RAFT_H_

#include <functional>
#include "config.h"
#include "raft_log.h"

namespace anraft {

typedef std::function<void()> TickFunc_t;
// StateType represents the role of a node in a cluster.
typedef  uint64_t StateType;

class Raft {
public:
    static Raft& NewRaft(const Config& config);
    void BecomeFollower(uint64_t term, uint64_t lead);

    void AddNode(uint64_t id);
private:
    Raft(Config& config);

    // TickElection is run by followers and candidates after electionTimeout.
    void TickElection();

private:
    uint64_t id_;
    TickFunc_t tick_;

    RaftLog& raftlog_;
};

} //namespace anraft

#endif //SOURCE_DIRECTORY_RAFT_H_