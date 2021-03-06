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

#ifndef SOURCE_DIRECTORY__SRC_EXAMPLES_HTTPAPI__H_
#define SOURCE_DIRECTORY__SRC_EXAMPLES_HTTPAPI__H_

#include <future>
#include <utility>
#include <butil/logging.h>
#include <brpc/server.h>
#include <brpc/restful.h>
#include "example_http.pb.h"
#include "proto/raft.pb.h"

#include "../raftexample/kvstore.h"



namespace example {

// Handler for a http based key-value store backed by raft
class HttpApi : public HttpService {
public:
	static HttpApi& Instance();
    void ServeHttpKVAPI(int port);
    
    void Post(google::protobuf::RpcController* cntl_base,
                const HttpRequest*,
                HttpResponse*,
                google::protobuf::Closure* done);

    void Put(google::protobuf::RpcController* cntl_base,
                const HttpRequest* request,
                HttpResponse*,
                google::protobuf::Closure* done);

    void Get(google::protobuf::RpcController* cntl_base,
                const HttpRequest* request,
                HttpResponse*,
                google::protobuf::Closure* done);				
private:
	HttpApi();

private:
    KvStore& kvstore_;
};


} //namespace example

#endif //SOURCE_DIRECTORY__SRC_EXAMPLES_HTTPAPI__H_
