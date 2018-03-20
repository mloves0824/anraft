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
#include <queue>
#include "config.h"
#include "raft_log.h"
#include "progress.h"
#include "proto/raft.pb.h"

namespace anraft {

class Raft;
typedef std::function<void(void)> TickFunc_t;
typedef std::function<RaftError(Raft*, Message*)> StepFunc_t;
// StateType represents the role of a node in a cluster.
// Possible values for StateType.
enum StateType {
    StateFollower = 0,
    StateCandidate,
    StateLeader,
    StatePreCandidate,
};


// Possible values for CampaignType
enum CampaignType {
    // campaignPreElection represents the first phase of a normal election when
    // Config.PreVote is true.
    CampaignPreElection = 0,
    // campaignElection represents a normal (time-based) election (the second phase
    // of the election when Config.PreVote is true).
    CampaignElection,
    // campaignTransfer represents the type of leader transfer
    CampaignTransfer
};



class Raft {
public:
    static Raft& NewRaft(const Config& config);
    static Raft& GetRaft();
    void BecomeFollower(uint64_t term, uint64_t lead);
    void BecomeCandidate();

    void AddNode(uint64_t id);
    uint64_t GetID() const { return id_; }

    RaftError Step(Message* msg);

private:
    Raft(Config& config);

    // TickElection is run by followers and candidates after electionTimeout.
    void TickElection();
    static RaftError StepFollower(Raft*, Message*);
    void Reset(uint64_t term);
    bool Promotable();
    bool PastElectionTimeout();
    void Campaign(CampaignType t);
    bool Quorum();
    bool Poll();
    void Send(Message&);


private:
    uint64_t id_;
    uint64_t term_;
    StateType state_;
    bool pre_vote_;
    std::map<uint64_t, Progress> prs_;

    std::queue<Message> msgs_;

    TickFunc_t tick_func_;
    StepFunc_t step_func_;

    // number of ticks since it reached last electionTimeout when it is leader
    // or candidate.
    // number of ticks since it reached last electionTimeout or received a
    // valid message from current leader when it is a follower.
    int election_elapsed_;

    RaftLog& raftlog_;
};

} //namespace anraft

#endif //SOURCE_DIRECTORY_RAFT_H_
