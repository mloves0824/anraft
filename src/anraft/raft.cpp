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

#include <vector>
#include <algorithm>
#include "raft.h"
#include "butil/rand_util.h"

namespace anraft {

Raft& Raft::NewRaft(const Config& config) {}

Raft& Raft::GetRaft() {}

void Raft::BecomeFollower(uint64_t term, uint64_t lead) {
    step_func_ = Raft::StepFollower;
    Reset(term);
    tick_func_ = std::bind(&Raft::TickElection, this);
    lead_ = lead;
    state_ = StateFollower;
    //r.logger.Infof("%x became follower at term %d", r.id, r.Term) TODO
}


void Raft::BecomeCandidate() {
    if (state_ == StateLeader) {
        ///TODO:panic("invalid transition [leader -> candidate]")
    }
    step_func_ = Raft::StepCandidate;
    Reset(term_ + 1);
    tick_func_ = std::bind(&Raft::TickElection, this);
    vote_ = id_;
    state_ = StateFollower;
    //r.logger.Infof("%x became candidate at term %d", r.id, r.Term)
}

void Raft::BecomeLeader() {
    if (state_ == StateFollower) {
        //TODO: panic("invalid transition [follower -> leader]")
    }

    step_func_ = Raft::StepLeader;
    Reset(term_);
    tick_func_ = std::bind(&Raft::TickHeatbeat, this);
    lead_ = id_;
    state_ = StateLeader;

    //TODO: raftlog process
    //ents, err : = r.raftLog.entries(r.raftLog.committed + 1, noLimit)
    //if err != nil{
    //    r.logger.Panicf("unexpected error getting uncommitted entries (%v)", err)
    //}

    //// Conservatively set the pendingConfIndex to the last index in the
    //// log. There may or may not be a pending config change, but it's
    //// safe to delay any future proposals until we commit all our
    //// pending log entries, and scanning the entire tail of the log
    //// could be expensive.
    //if len(ents) > 0 {
    //    r.pendingConfIndex = ents[len(ents) - 1].Index
    //}

    //    r.appendEntry(pb.Entry{ Data: nil })
    //        r.logger.Infof("%x became leader at term %d", r.id, r.Term)
}

void Raft::BcastAppend() {
    ForEachProgress([=](uint64_t id, Progress p)
    {
        if (id = this->GetID())
            return;
        this->SendAppend(id);
    });
}

void Raft::BcastHeartbeat() {
    std::string ctx = ReadOnly::Instance().LastPendingRequestCtx();
    BcastHeartbeatWithCtx(ctx);
}

void Raft::BcastHeartbeatWithCtx(const std::string& ctx) {
    ForEachProgress([=](uint64_t id, Progress p)
    {
        if (id = this->GetID())
            return;
        this->SendHeartbeat(id, ctx);
    });
}

void Raft::ForEachProgress(std::function<void(uint64_t, Progress)> f) {
    for (auto &x : prs_) {
        f(x.first, x.second);
    }

    for (auto &x : learner_prs_) {
        f(x.first, x.second);
    }
}

RaftError Raft::StepFollower(Raft* raft, Message* msg) {

	if (!raft || !msg) {
		return ErrParam;
	}

	switch (msg->type()) {
	case MsgApp:
		raft->election_elapsed_ = 0;
		raft->lead_ = msg->from();
		raft->HandleAppendEntries(msg);
		break;
	}
}
RaftError Raft::StepCandidate(Raft* raft, Message* msg) {
    // Only handle vote responses corresponding to our candidacy (while in
    // StateCandidate, we may get stale MsgPreVoteResp messages in this term from
    // our pre-candidate state).
    MessageType my_vote_resp_type;
    if (raft->state_ == StatePreCandidate) {
        my_vote_resp_type = MsgPreVoteResp;
    } else {
        my_vote_resp_type = MsgVoteResp;
    }

    switch (msg->type()) {
    case MsgPreVoteResp:
    case MsgVoteResp:
        int grant = raft->Poll(msg->from(), msg->type(), !msg->reject());
        //    r.logger.Infof("%x [quorum:%d] has received %d %s votes and %d vote rejections", r.id, r.quorum(), gr, m.Type, len(r.votes) - gr)
        if (grant > raft->Quorum()) {
            if (raft->state_ == StatePreCandidate) {
                //TODO: r.campaign(campaignElection)
            } else {
                raft->BecomeLeader();
                raft->BcastAppend();
            }
        } 
        else {
            // pb.MsgPreVoteResp contains future term of pre-candidate
            // m.Term > r.Term; reuse r.Term
            raft->BecomeFollower(raft->term_, 0);
        }
        break;
    }
}


RaftError Raft::StepLeader(Raft* raft, Message* msg) {

    // These message types do not require any progress for m.From.
    switch (msg->type()) {
    case MsgBeat:
        break;
    case MsgCheckQuorum:
        if (!raft->CheckQuorumActive()) {
            //r.logger.Warningf("%x stepped down to follower since quorum is not active", r.id)
            raft->BecomeFollower(raft->term_, 0);
        }
        break;
    case MsgProp:
        if (msg->entries().empty()) {} //TODO.logger.Panicf("%x stepped empty MsgProp", r.id)
        if (raft->prs_.find(raft->id_) == raft->prs_.end()) {
            // If we are not currently a member of the range (i.e. this node
            // was removed from the configuration while serving as leader),
            // drop any new proposals.
            return anraft::ErrProposalDropped;
        }
        if (raft->lead_transferee_ != 0) {
            //r.logger.Debugf("%x [term %d] transfer leadership to %x is in progress; dropping proposal", r.id, r.Term, r.leadTransferee)
            return anraft::ErrProposalDropped;
        }
        //TODO: EntryConfChange process
        //    for i, e : = range m.Entries{
        //        if e.Type == pb.EntryConfChange{
        //            if r.pendingConfIndex > r.raftLog.applied{
        //                r.logger.Infof("propose conf %s ignored since pending unapplied configuration [index %d, applied %d]",
        //                e.String(), r.pendingConfIndex, r.raftLog.applied)
        //                m.Entries[i] = pb.Entry{ Type: pb.EntryNormal }
        //            }
        //            else {
        //                    r.pendingConfIndex = r.raftLog.lastIndex() + uint64(i) + 1
        //                }
        //        }
        //    }
        raft->AppendEntry(const_cast<::google::protobuf::RepeatedPtrField< ::anraft::LogEntry >& >(msg->entries()));
        raft->BcastAppend();
        break;
    }

//	// All other message types require a progress for m.From (pr).
    auto pr = raft->GetProgress(msg->from());
    if (!pr) {
    	//		r.logger.Debugf("%x no progress available for %x", r.id, m.From)
    	return ErrParam;
    }

    switch (msg->type()) {
    case MsgAppResp:
    	if (msg->reject()) {}
    	else {}
    	break;

    }
}

void Raft::Reset(uint64_t term) { //TODO


    ResetRandomizedElectionTimeout();
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

    //term == term_
    switch (msg->type()) {
    case MsgHup:
        if (state_ != StateLeader) {
            //TODO: read from raftlog

            //TODO: r.logger.Infof("%x is starting a new election at term %d", r.id, r.Term)
            if (pre_vote_) {
                //TODO: r.campaign(campaignPreElection)
            }
            else
            {
                Campaign(CampaignElection);
            }
        }
        else {
            //TODO: r.logger.Debugf("%x ignoring MsgHup because already leader", r.id) 
        }
        break;

    case MsgVote:
    case MsgPreVote:
    	if (is_learner_) {
			// TODO: learner may need to vote, in case of node down when confchange.
			// TODO: r.logger.Infof("%x [logterm: %d, index: %d, vote: %x] ignored %s from %x [logterm: %d, index: %d] at term %d: learner can not vote",
			//	r.id, r.raftLog.lastTerm(), r.raftLog.lastIndex(), r.Vote, m.Type, m.From, m.LogTerm, m.Index, r.Term)
			return ErrNone;
    	}

		// The m.Term > r.Term clause is for MsgPreVote. For MsgVote m.Term should
		// always equal r.Term.
    	if ((vote_ == 0 || vote_ == msg->from() || msg->term() > term_) && raftlog_.IsUpToDate(msg->index(), msg->term())) {
//		TODO:	r.logger.Infof("%x [logterm: %d, index: %d, vote: %x] cast %s for %x [logterm: %d, index: %d] at term %d",
//				r.id, r.raftLog.lastTerm(), r.raftLog.lastIndex(), r.Vote, m.Type, m.From, m.LogTerm, m.Index, r.Term)
			// When responding to Msg{Pre,}Vote messages we include the term
			// from the message, not the local term. To see why consider the
			// case where a single node was previously partitioned away and
			// it's local term is now of date. If we include the local term
			// (recall that for pre-votes we don't update the local term), the
			// (pre-)campaigning node on the other end will proceed to ignore
			// the message (it ignores all out of date messages).
			// The term in the original message and current local term are the
			// same in the case of regular votes, but different for pre-votes.
    		Message resp_msg;
    		resp_msg.set_to(msg->from());
    		resp_msg.set_term(msg->term());
    		resp_msg.set_type(VoteRespMsgType(msg->type()));
    		if (msg->type() == MsgVote) {
    			vote_ = msg->from();
    			election_elapsed_ = 0;
    		}
    	}
    	break;

    default:
        return step_func_(this, msg);
    }

    return ErrNone;
}

void Raft::Tick() {
    tick_func_();
}


bool Raft::IsResponseMsg(MessageType msgt) {
    return (msgt == MsgAppResp || msgt == MsgVoteResp || msgt == MsgHeartbeatResp || msgt == MsgUnreachable || msgt == MsgPreVoteResp);
}

void Raft::TickElection() {
    election_elapsed_++;

    if (Promotable() && PastElectionTimeout()) {
        election_elapsed_ = 0;
        Message msg;
        msg.set_type(MsgHup);
        msg.set_from(id_);
        Step(&msg);
    }
}

void Raft::TickHeatbeat(void) {
    election_elapsed_++;
    heartbeat_elapsed_++;

    if (election_elapsed_ > election_timeout_) {
        election_elapsed_ = 0;
        if (check_quorum_) {
            Message msg;
            msg.set_type(MsgCheckQuorum);
            msg.set_from(id_);
            Step(&msg);
        }
        //TODO: If current leader cannot transfer leadership in electionTimeout, it becomes leader again.
    }

    if (state_ != StateLeader) {
        //TODO: log
        return;
    }

    if (heartbeat_elapsed_ > heartbeat_timeout_) {
        heartbeat_elapsed_++;
        Message msg;
        msg.set_type(MsgBeat);
        msg.set_from(id_);
        Step(&msg);
    }
}


int Raft::Poll(uint64_t id, MessageType type, bool vote) {
    if (vote) {
        //r.logger.Infof("%x received %s from %x at term %d", r.id, t, id, r.Term)
    } else {
        //r.logger.Infof("%x received %s rejection from %x at term %d", r.id, t, id, r.Term)
    }

    if (votes_.find(id) == votes_.end())
        votes_[id] = vote;

    int grant;
    for (auto &x : votes_) {
        if (x.second)
            grant++;
    }

    return grant;
}


Progress* Raft::GetProgress(uint64_t id) {
    if (prs_.find(id) != prs_.end()) {
        return &prs_[id];
    }

    return &learner_prs_[id]; //TODO: empty process
}


void Raft::Campaign(CampaignType t) {
    uint64_t term;
    MessageType msg_type;

    if (t == CampaignPreElection) {} //TODO: CampaignPreElection process
    else {
        BecomeCandidate();
        msg_type = MsgVote;
        term = term_;
    }

    if (Quorum() && Poll()) { //TODO: single-node cluster
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

bool Raft::Promotable() {
    return (prs_.find(id_) != prs_.end());
}

bool Raft::PastElectionTimeout() {
    return election_elapsed_ >= randomized_election_timeout_;
}

void Raft::ResetRandomizedElectionTimeout() {
    randomized_election_timeout_ = election_timeout_ + butil::RandGenerator(election_timeout_);
}


bool Raft::Poll() {}



void Raft::Send(Message& msg) {
    msg.set_from(id_);

    msgs_.push(msg);
}

void Raft::SendHeartbeat(uint64_t to, const std::string& ctx) {
    // Attach the commit as min(to.matched, r.committed).
    // When the leader sends out heartbeat message,
    // the receiver(follower) might not be matched with the leader
    // or it might not have all the committed entries.
    // The leader MUST NOT forward the follower's commit to
    // an unmatched index.
    uint64_t commit;
    //TODO: commit: = min(r.getProgress(to).Match, r.raftLog.committed)

    Message msg;
    msg.set_to(to);
    msg.set_type(MsgHeartbeat);
    msg.set_commit(commit);
    msg.set_context(ctx);
    this->Send(msg);
}

void Raft::SendAppend(uint64_t to) {
    Progress* pr = GetProgress(to);
    if (!pr)
        return;
    if (pr->IsPaused())
        return;

    Message msg;
    msg.set_to(to);

    auto term_result = raftlog_.Term(pr->Next() - 1);
    RaftError term_error = std::get<0>(term_result);
    uint64_t term = std::get<1>(term_result);

    std::vector<LogEntry> entries;
    RaftError entries_result = raftlog_.Entries(pr->Next(), max_msg_size_, entries);

    if (term_error != ErrNone || entries_result != ErrNone) {
    	//TODO: // send snapshot if we failed to get term or entries
    }
    else {
    	msg.set_type(MsgApp);
    	msg.set_index(pr->Next() - 1);
    	msg.set_term(term);
    	//msg.add_entries()->set   //TODO: add_entries
    	msg.set_commit(raftlog_.Committed());
    	if (!msg.entries().empty()) {
    		auto state = pr->State();
    		//if (state == ProgressStateProbe) //TODO: Progress processing
    	}
    }

    Send(msg);
}


bool Raft::CheckQuorumActive() {

}

void Raft::AppendEntry(::google::protobuf::RepeatedPtrField< ::anraft::LogEntry >& entries) {
	uint64_t li = raftlog_.LastIndex();

	int i = 0;
	for (auto iter = entries.begin(); iter != entries.end(); iter++, i++) {
		iter->set_term(term_);
		iter->set_index(li + 1 + i);
	}

    // use latest "last" index after truncate/append
    li = raftlog_.Append(entries);  //TODO
    GetProgress(id_)->MaybeUpdate(li);
    // Regardless of maybeCommit's return, our caller will call bcastAppend.
    MaybeCommit();
}

MessageType Raft::VoteRespMsgType(MessageType vote) {
	if (vote == MsgVote) {
		return MsgVoteResp;
	} else
		return MsgPreVoteResp;
}

//func (r *raft) maybeCommit() bool {
//	// TODO(bmizerany): optimize.. Currently naive
//	mis := make(uint64Slice, 0, len(r.prs))
//	for _, p := range r.prs {
//		mis = append(mis, p.Match)
//	}
//	sort.Sort(sort.Reverse(mis))
//	mci := mis[r.quorum()-1]
//	return r.raftLog.maybeCommit(mci, r.Term)
//}


bool Raft::MaybeCommit() {
	// TODO(bmizerany): optimize.. Currently naive
	std::vector<uint64_t> vp;
	for (auto &x : prs_) {
		vp.push_back(x.second.Match());
	}

	std::sort(vp.begin(), vp.end(), std::greater<uint64_t>());  //TODO: Reverse???
	uint64_t index = vp[Quorum() - 1];
	return raftlog_.MaybeCommit(index, term_);
}

int Raft::Quorum() {
	return prs_.size() / 2 + 1;
}

void Raft::HandleAppendEntries(Message* msg) {
	if (msg->index() < raftlog_.Committed()) {
		Message resp;
		resp.set_to(msg->from());
		resp.set_type(MsgAppResp);
		resp.set_index(raftlog_.Committed());
		Send(resp);
	}

	auto ma_ret = raftlog_.MaybeAppend(msg->index(), msg->term(), msg->commit(), const_cast<PbVectorLogentryType&>(msg->entries()));
	if (std::get<1>(ma_ret) != ErrNone) {
		Message resp;
		resp.set_to(msg->from());
		resp.set_type(MsgAppResp);
		resp.set_index(std::get<0>(ma_ret));
		Send(resp);
	}
	else {
		//		r.logger.Debugf("%x [logterm: %d, index: %d] rejected msgApp [logterm: %d, index: %d] from %x",
		//			r.id, r.raftLog.zeroTermOnErrCompacted(r.raftLog.term(m.Index)), m.Index, m.LogTerm, m.Index, m.From)
		Message resp;
		resp.set_to(msg->from());
		resp.set_type(MsgAppResp);
		resp.set_index(msg->index());
		resp.set_reject(true);
		resp.set_rejecthint(raftlog_.LastIndex());
		Send(resp);
	}
}




} //namespace anraft
