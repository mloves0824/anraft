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
#include "read_only.h"

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
    void BecomeLeader();

    // bcastAppend sends RPC, with entries to all peers that are not up-to-date
    // according to the progress recorded in r.prs.
    void BcastAppend();
    // bcastHeartbeat sends RPC, without entries to all the peers.
    void BcastHeartbeat();
    void BcastHeartbeatWithCtx(const std::string& ctx);

    void AddNode(uint64_t id);
    uint64_t GetID() const { return id_; }

    RaftError Step(Message* msg);
    void Tick();
    Progress* GetProgress(uint64_t id);
    int Poll(uint64_t id, MessageType type, bool vote);
    
    //utils
    static bool IsResponseMsg(MessageType type);

    void Send(Message&);
    // sendHeartbeat sends an empty MsgApp
    void SendHeartbeat(uint64_t to, const std::string& ctx);
    // sendAppend sends RPC, with entries to the given peer.
    void SendAppend(uint64_t to);
private:
    Raft(Config& config);

    // TickElection is run by followers and candidates after electionTimeout.
    void TickElection();
    void TickHeatbeat(void);
    static RaftError StepFollower(Raft*, Message*);
    // stepCandidate is shared by StateCandidate and StatePreCandidate; the difference is
    // whether they respond to MsgVoteResp or MsgPreVoteResp.
    static RaftError StepCandidate(Raft*, Message*);
    static RaftError StepLeader(Raft*, Message*);


    void Reset(uint64_t term);
    // promotable indicates whether state machine can be promoted to leader,
    // which is true when its own id is in progress list.
    bool Promotable();
    // pastElectionTimeout returns true if r.electionElapsed is greater
    // than or equal to the randomized election timeout in
    // [electiontimeout, 2 * electiontimeout - 1].
    bool PastElectionTimeout();
    void ResetRandomizedElectionTimeout();
    void Campaign(CampaignType t);
    bool Quorum();
    bool Poll();


    // checkQuorumActive returns true if the quorum is active from
    // the view of the local raft state machine. Otherwise, it returns
    // false.
    // checkQuorumActive also resets all RecentActive to false.
    bool CheckQuorumActive();

    void AppendEntry(::google::protobuf::RepeatedPtrField< ::anraft::LogEntry >& entries);

    void ForEachProgress(std::function<void(uint64_t, Progress)>);
    static MessageType VoteRespMsgType(MessageType vote);
private:
    uint64_t id_;
    uint64_t term_;
    uint64_t vote_;
    StateType state_;
    bool pre_vote_;
    bool check_quorum_;
    bool is_learner_;
    std::map<uint64_t, Progress> prs_;
    std::map<uint64_t, Progress> learner_prs_;
    std::map<uint64_t, bool> votes_;

    std::queue<Message> msgs_;

    // the leader id
    uint64_t lead_;
    // leadTransferee is id of the leader transfer target when its value is not zero.
    // Follow the procedure defined in raft thesis 3.10.
    uint64_t lead_transferee_;

    TickFunc_t tick_func_;
    StepFunc_t step_func_;

    // number of ticks since it reached last electionTimeout when it is leader
    // or candidate.
    // number of ticks since it reached last electionTimeout or received a
    // valid message from current leader when it is a follower.
    int election_elapsed_;

    // number of ticks since it reached last heartbeatTimeout.
    // only leader keeps heartbeatElapsed.
    int heartbeat_elapsed_;

    int heartbeat_timeout_;
    int election_timeout_;
    // randomizedElectionTimeout is a random number between
    // [electiontimeout, 2 * electiontimeout - 1]. It gets reset
    // when raft changes its state to follower or candidate.
    int randomized_election_timeout_;

    RaftLog& raftlog_;
};

} //namespace anraft

#endif //SOURCE_DIRECTORY_RAFT_H_
