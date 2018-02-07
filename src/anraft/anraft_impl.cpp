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

#include <functional>
#include "brpc/server.h"
#include "bthread/bthread.h"
#include "bthread/unstable.h"
#include "anraft_impl.h"
#include "butil/time/time.h"

namespace anraft {

AnraftImpl::AnraftImpl() : rpc_client_(AnraftNodeClientPtr(new AnraftNodeClient(options_.nodes))) {
	if (Recover(options_.db_path)) {
		LOG(ERROR) << "Fail to recover.";
		return;
	}

	bthread_timer_t timer_id = 0;
	int rc = bthread_timer_add(&timer_id, 
		                       butil::microseconds_to_timespec(election_timeout_), 
					           std::bind(&AnraftImpl::Election, 
					           this, std::placeholders::_1), NULL);
	if (rc) {
		LOG(ERROR) << "Fail to add timer: " << berror(rc);
		return;
	}

	for (int i = 0; i < follower_contexts_.size(); i++) {
		if (bthread_start_background(&(follower_contexts_[i]->tid), NULL, std::bind(&AnraftImpl::ReplicateLog, this, (void*)&i), NULL) != 0) {
			LOG(ERROR) << "Fail to create bthread";
			return;
		}
	}

}

AnraftImpl::~AnraftImpl() {}

void AnraftImpl::Vote(::google::protobuf::RpcController* controller,
                      const ::anraft::VoteRequest* request,
	                  ::anraft::VoteResponse* response,
	                  ::google::protobuf::Closure* done) {
	// This object helps you to call done->Run() in RAII style. If you need
	// to process the request asynchronously, pass done_guard.release().
	brpc::ClosureGuard done_guard(done);

}


void AnraftImpl::Election(void*) {
	if (role_ == kLeader) {
		LOG(NOTICE) << "Fail to Recover: log_ is null.";
		return;
	}

	current_term_++;
	role_ = kCandidate;
	voted_for_ = options_.local_addr;
	LOG(INFO) << "Start Election: current_term=%s" << current_term_;

	VoteRequest* request = new VoteRequest;
	VoteResponse* response = new VoteResponse;
	request->set_term(current_term_);
	request->set_candidate_id(options_.local_addr);
	request->set_last_log_term(last_log_term_);
	request->set_last_log_index(last_log_index_);
	std::function<void(const VoteRequest* request, VoteResponse* response, bool, int, const std::string&)> callback = 
					  std::bind(&AnraftImpl::ElectionCallback, this, request, response, 
					  std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
	if (!rpc_client_->SendRequest(&RaftNode_Stub::Vote, request, response, callback)) {
		LOG(NOTICE) << "Fail to SendRequest.";
		return;
	}

	bthread_timer_t timer_id = 0;
	int rc = bthread_timer_add(&timer_id,
		                       butil::microseconds_to_timespec(election_timeout_),
		                       std::bind(&AnraftImpl::Election,
		                       this, std::placeholders::_1), NULL);
	if (rc) {
		LOG(ERROR) << "Fail to add timer: " << berror(rc);
		return;
	}
}


void AnraftImpl::ElectionCallback(const ::anraft::VoteRequest* request,
	                              ::anraft::VoteResponse* response,
	                              bool failed,
	                              int error,
								  const std::string &vote_node) {
	if (!failed || error) {
		LOG(ERROR) << "ElectionCallback: failed=" << failed << ",error=" << error;
		return;
	}

	int64_t term = response->term();
	bool granted = response->vote_granted();
	if (term > current_term_) {
		if (role_ == kLeader) {
			LOG(WARNING) << "Leader change to Follower";
		} else {
			LOG(WARNING) << "Change role to Follower";
		}
		current_term_ = term;
		voted_for_ = "";
		//TODO update db
		role_ = kFollower;
		ResetElection();
	}

	if (term != current_term_ || !granted || role_ == kLeader) {
		return;
	}

	votes_.insert(vote_node);
	if (votes_.size() >= (options_.nodes.size() / 2 + 1)) {
		role_ = kLeader;
		leader_ = options_.local_addr;
		//TODO update db
		//TODO cancel election
		//TODO start heartbeat
	}
}


bool AnraftImpl::Recover(const std::string& db_path) {
	// Create DB
	if (!log_) {
		LOG(ERROR) << "Fail to Recover: log_ is null." ;
		return false;
	}

	if (!log_->Open(db_path)) {
		LOG(ERROR) << "Fail to Recover: Open error.";
		return false;
	}

	current_term_ = log_->GetCurrentTerm();
}



void AnraftImpl::ResetElection() {
	bthread_timer_t timer_id = 0;
	int rc = bthread_timer_add(&timer_id,
		                       butil::microseconds_to_timespec(election_timeout_),
		                       std::bind(&AnraftImpl::Election,
		                       this, std::placeholders::_1), NULL);
	if (rc) {
		LOG(ERROR) << "Fail to add timer: " << berror(rc);
		return;
	}
}


void AnraftImpl::ReplicateLog(void *arg) {
	int id = 0;
	if (arg) {
		id = *(int*)arg;
	}
	FollowerContext *follower = follower_contexts_[id];

	while (true) {
		
		if (is_stop_) {
			return;
		}
		while (role_ != kLeader) {
			follower->conditon.Wait();
		}

		ReplicateLogToFollower(id);
		int64_t time_replica;
		follower->conditon.TimedWait(butil::TimeDelta::FromMicroseconds(time_replica));
	}
}


void AnraftImpl::ReplicateLogToFollower(uint32_t id) {

}


}

