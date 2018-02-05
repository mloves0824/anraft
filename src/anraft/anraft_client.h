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

#include <memory>
#include <vector>
#include <functional>
#include "brpc/channel.h"
#include "raft.pb.h"

namespace anraft {

class AnraftNodeClient {
public:
	AnraftNodeClient(const std::vector<std::string> &nodes);
	~AnraftNodeClient();

	template <class Request, class Response, class Callback> 
	bool SendRequest(void(RaftNode_Stub::*func)(google::protobuf::RpcController*,
												Request*, Response*, Callback*),
					 Request* request, Response *response, 
					 std::function<void(Request*, Response*, bool, int)> callback,
					 int64_t timeout = 300, int retry_times = 1) {
		for (auto &x : stubs_) {
			if (x.second) {
				brpc::Controller cntl;
				if (callback == nullptr) {
					(x.second->*func)(&cntl, request, response, NULL);
				}
				else {
					// We use protobuf utility `NewCallback' to create a closure object
					// that will call our callback `HandleEchoResponse'. This closure
					// will automatically delete itself after being called once
					google::protobuf::Closure* done = brpc::NewCallback(
						&AnraftNodeClient::template RpcCallBack<Request, Response, Callback>, cntl, request, response, callback);
					brpc::Controller *cntl = new brpc::Controller();
					(x.second->*func)(&cntl, request, response, done);
				}
			}
		}
	}

	template <class Request, class Response, class Callback> 
	static void RpcCallBack(brpc::Controller *cntl,
						    Request *req,
							Response *resp,
							std::function<void(Request*, Response*, bool, int)> callback) {
	
	}

private:
	// A Channel represents a communication line to a Server. Notice that 
	// Channel is thread-safe and can be shared by all threads in your program.
	std::map<std::string, RaftNode_Stub*> stubs_;
};

typedef std::shared_ptr<AnraftNodeClient> AnraftNodeClientPtr;
}