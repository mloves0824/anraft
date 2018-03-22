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

#include "stream.h"

namespace rafthttp {

bool StreamWriter::Start(const std::string& url) {
	StreamWriterPtr stream(new StreamWriter());

    // A Channel represents a communication line to a Server. Notice that
    // Channel is thread-safe and can be shared by all threads in your program.
    brpc::ChannelOptions options;
    options.protocol = "http";
    stream->url_ = url;

    // Initialize the channel, NULL means using default options.
    // options, see `brpc/channel.h'.
    if (stream->channel_.Init(url.c_str(), "", &options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        return -1;
    }

    bthread::execution_queue_start(stream->send_queue_id_,
                                    NULL,
									StreamWriter::ServeSendMsg,
                                    (void*)stream.get());


}


int StreamWriter::ServeSendMsg(void* meta, bthread::TaskIterator<anraft::Message>& iter) {
	StreamWriter* stream = (StreamWriter*)meta;
    if (iter.is_queue_stopped()) {
        return 0;
    }

    for (; iter; ++iter) {
    	std::string msg;
    	iter->SerializeToString(&msg);
        // We will receive response synchronously, safe to put variables
        // on stack.
        brpc::Controller cntl;

        cntl.http_request().uri() = stream->url_;
        if (!msg.empty()) {
            cntl.http_request().set_method(brpc::HTTP_METHOD_GET);
            cntl.request_attachment().append(msg);
        }

		// Because `done'(last parameter) is NULL, this function waits until
		// the response comes back or error occurs(including timedout).
        stream->channel_.CallMethod(NULL, &cntl, NULL, NULL, NULL);
		if (cntl.Failed()) {
			std::cerr << cntl.ErrorText() << std::endl;
			return -1;
		}

    }

}

bool StreamReader::Start(int port) {

    // Generally you only need one Server.
    brpc::Server server;
    rafthttp::StreamReader service;
    // Add services into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(&service,
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add http_svc";
        return -1;
    }

    // Start the server.
    brpc::ServerOptions options;
    if (server.Start(port, &options) != 0) {
        LOG(ERROR) << "Fail to start HttpServer";
        return -1;
    }

    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    return true;
}


void StreamReader::Get(google::protobuf::RpcController* cntl_base,
	     const HttpRequest*,
	     HttpResponse*,
	     google::protobuf::Closure* done) {//TODO


}


} //namespace rafthttp
