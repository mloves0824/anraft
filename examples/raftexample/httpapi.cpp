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

#include "httpapi.h"

namespace example {

HttpApi& HttpApi::Instance() {
	static HttpApi g_httpapi;
	return g_httpapi;
}

HttpApi::HttpApi() : kvstore_(KvStore::Instance()) {};

void HttpApi::ServeHttpKVAPI(int port) {
    //start http server
    brpc::Server server;

    // Add services into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(this, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add http_svc";
        return;
    }

    // Start the server.
    brpc::ServerOptions options;
    if (server.Start(port, &options) != 0) {
        LOG(ERROR) << "Fail to start HttpServer";
        return;
    }

    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();

    ////wait for error
    //anraft::RaftError error = future_error.get();
    //if (error != anraft::ErrNone) {
    //    LOG(ERROR) << "Exit when: " << error;
    //}
}


void HttpApi::Post(google::protobuf::RpcController* cntl_base,
                    const HttpRequest*,
                    HttpResponse*,
                    google::protobuf::Closure* done) {
    // This object helps you to call done->Run() in RAII style. If you need
    // to process the request asynchronously, pass done_guard.release().
    brpc::ClosureGuard done_guard(done);

    brpc::Controller* cntl =
        static_cast<brpc::Controller*>(cntl_base);

    cntl->http_request().uri();
    anraft::ConfChange conf_change;
    conf_change.set_id(1); //TODO

    //notify future
    promise_confchange_.set_value(conf_change);

    // Fill response.
    cntl->http_response().set_status_code(brpc::HTTP_STATUS_NO_CONTENT);
}


void HttpApi::Put(google::protobuf::RpcController* cntl_base,
                    const HttpRequest* request,
                    HttpResponse*,
                    google::protobuf::Closure* done) {
    // This object helps you to call done->Run() in RAII style. If you need
    // to process the request asynchronously, pass done_guard.release().
    brpc::ClosureGuard done_guard(done);

    brpc::Controller* cntl =
        static_cast<brpc::Controller*>(cntl_base);

    std::string key;
    std::string value(request->message());

    kvstore_->Propose(key, value);

    //notify future
    //promise_confchange_.set_value(conf_change); TODO

    // Fill response.
    cntl->http_response().set_status_code(brpc::HTTP_STATUS_NO_CONTENT);
}


void HttpApi::Get(google::protobuf::RpcController* cntl_base,
					const HttpRequest* request,
					HttpResponse*,
					google::protobuf::Closure* done) {
	// This object helps you to call done->Run() in RAII style. If you need
    // to process the request asynchronously, pass done_guard.release().
    brpc::ClosureGuard done_guard(done);

    brpc::Controller* cntl =
        static_cast<brpc::Controller*>(cntl_base);

    std::string key;
    std::string value(request->message());
	kvstore_.Lookup(key, value);
}

					

} //namespace example
