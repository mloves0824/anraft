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
#include <bthread/execution_queue.h>
#include <brpc/channel.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include <brpc/restful.h>
#include "proto/raft.pb.h"
#include "pb/http.pb.h"


#ifndef SOURCE_DIRECTORY_RAFTHTTP_STREAM_H_
#define SOURCE_DIRECTORY_RAFTHTTP_STREAM_H_

namespace rafthttp {

class StreamWriter {
public:
    static bool Start(const std::string& url);
    static int ServeSendMsg(void* meta, bthread::TaskIterator<anraft::Message>& iter);

private:
    bthread::ExecutionQueueId<anraft::Message>* send_queue_id_;
    brpc::Channel channel_;
    std::string url_;

};

typedef std::shared_ptr<StreamWriter> StreamWriterPtr;

class StreamReader :rafthttp::StreamHttpService {
public:
    static bool Start(int port);
    void Get(google::protobuf::RpcController* cntl_base,
		     const HttpRequest*,
		     HttpResponse*,
		     google::protobuf::Closure* done);

private:
    bthread::ExecutionQueueId<anraft::Message>* send_queue_id_;
    brpc::Channel channel_;
    std::string url_;

};

typedef std::shared_ptr<StreamReader> StreamReaderPtr;

} //namespace rafthttp

#endif //SOURCE_DIRECTORY_RAFTHTTP_STREAM_H_
