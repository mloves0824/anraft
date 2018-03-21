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

#ifndef SOURCE_DIRECTORY__SRC_EXAMPLES_KVSTORE__H_
#define SOURCE_DIRECTORY__SRC_EXAMPLES_KVSTORE__H_

#include <memory>
#include <string>
#include <future>
#include <map>
#include <mutex>
#include "raftsnap/snapshotter.h"
#include <bthread/execution_queue.h>
#include "kvstore.pb.h"


namespace example {

class KvStore;
typedef std::shared_ptr<KvStore> KvStorePtr;

struct KvStoreChannalMsg {
    std::string body;
};


class KvStore {
public:
    static KvStorePtr NewKVStore(raftsnap::SnapshotterPtr snapshotter, 
                                 std::promise<std::string> promise_propose,
                                 std::promise<std::string> promise_commit);
    void Propose(const std::string& key, const std::string& value);
private:
    static int ReadCommits(void* meta, bthread::TaskIterator<KvStoreChannalMsg>& iter);

private:
    std::promise<std::string> promise_propose_;
    raftsnap::SnapshotterPtr snapshotter_;
    std::map<std::string, std::string> kv_store_;
    std::mutex mutex_;

    bthread::ExecutionQueueId<KvStoreChannalMsg> queue_id_;

};


} //namespace example

#endif //SOURCE_DIRECTORY__SRC_EXAMPLES_KVSTORE__H_