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

#include "raft.h"

namespace anraft {

Raft& Raft::NewRaft(const Config& config) {}

Raft& Raft::GetRaft() {}

void Raft::BecomeFollower(uint64_t term, uint64_t lead) {
    step_func_ = Raft::StepFollower;
    Reset(term);
    tick_func_ = std::bind(&Raft::TickElection, this);
}


RaftError Raft::StepFollower(Raft*, Message*) {}

void Raft::Reset(uint64_t term) {

}

void Raft::AddNode(uint64_t id) {}

RaftError Raft::Step(Message* msg) {
    // Handle the message term, which may result in our stepping down to a follower.

    uint64_t term = msg->term();
    if (term == 0) {
        // local message
    }
    else if (term > term_) {
    
    }
    else {
        
    }

    switch (msg->type()) {
    case MsgHup:
        if (state_ != StateLeader) {
            
            if (pre_vote_) {
            }
            else
            {
                Campaign(CampaignElection);
            }
        }
        else {
            //r.logger.Debugf("%x ignoring MsgHup because already leader", r.id)
        }
        break;
    }

    return ErrNone;
}


void Raft::TickElection() {
    election_elapsed_++;

    if (Promotable() && PastElectionTimeout()) { //TODO
        election_elapsed_ = 0;
        Message msg;
        msg.set_type(MsgHup);
        msg.set_from(id_);
        Step(&msg);
    }
}


void Raft::Campaign(CampaignType t) {
    uint64_t term;
    MessageType msg_type;

    if (t == CampaignPreElection) {}
    else {
        BecomeCandidate();
        msg_type = MsgVote;
        term = term_;
    }

    if (Quorum() && Poll()) {
        return;
    }

    for (auto &x : prs_) {
        if (x.first == id_) continue;

        //r.logger.Infof("%x [logterm: %d, index: %d] sent %s request to %x at term %d", //TODO

        if (t == CampaignTransfer) {}

        Message msg;
        msg.set_term(term);
        msg.set_to(x.first);
        msg.set_type(msg_type);
        //msg.set_index();
        //msg.set_logterm();
        //msg.set_context();

        Send(msg);
    }

}

bool Raft::Promotable() {}
bool Raft::PastElectionTimeout() {}
bool Raft::Quorum() {}
bool Raft::Poll() {}

void Raft::BecomeCandidate() {}

void Raft::Send(Message& msg) {
    msg.set_from(id_);

    msgs_.push(msg);
}

} //namespace anraft
