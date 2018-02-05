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

#include <vector>
#include <string>
#include "raft.pb.h"
#include "rocksdb/db.h"
#include "anraft_client.h"
#include "anraft_options.h"
#include "raft_log.h"

namespace anraft {

class AnraftImpl : public RaftNode {
public:
	AnraftImpl();
	~AnraftImpl();

	void Vote(::google::protobuf::RpcController* controller,
		      const ::anraft::VoteRequest* request,
		      ::anraft::VoteResponse* response,
		      ::google::protobuf::Closure* done);
	void AppendEntries(::google::protobuf::RpcController* controller,
		               const ::anraft::AppendEntriesRequest* request,
		               ::anraft::AppendEntriesResponse* response,
		               ::google::protobuf::Closure* done);
private:
	void Election(void*);
	void ElectionCallback(VoteRequest* request,
		                  VoteResponse* response, 
						  bool failed,
						  int error);
	bool Recover(const std::string& db_path);

private:
	//Persistent state on all servers :
	//(Updated on stable storage before responding to RPCs)
	//	currentTerm latest term server has seen(initialized to 0
	//	            on first boot, increases monotonically)
	int64_t current_term_;

	//votedFor candidateId that received vote in current
	//	       term(or null if none)	std::string voted_for_;
	//log[] log entries; each entry contains command
	//	    for state machine, and term when entry
	//		was received by leader(first index is 1)	RaftLog* log_;
	//Volatile state on all servers :
	//commitIndex index of highest log entry known to be
	//	          committed(initialized to 0, increases
	//	          monotonically)
	int64_t commit_index_;

	//	lastApplied index of highest log entry applied to state
	//	            machine(initialized to 0, increases
	//	            monotonically)	int64_t last_applied_;
	//Volatile state on leaders :
	//(Reinitialized after election)
	//	nextIndex[] for each server, index of the next log entry
	//	            to send to that server(initialized to leader
	//	            last log index + 1)
	//	matchIndex[] for each server, index of highest log entry
	//	             known to be replicated on server
	//	             (initialized to 0, increases monotonically)
	struct FollowerContext {
		int64_t next_index;
		int64_t match_index;
	};
	std::vector<FollowerContext*> follower_contexts_;



	Role role_;
	std::string leader_;

	int64_t election_timeout_;

	AnraftOptions options_;
	AnraftNodeClientPtr rpc_client_;

};

}