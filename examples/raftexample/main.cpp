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

#include "gflags/gflags.h"
#include <future>

#include "httpapi.h"
#include "kvstore.h"
#include "raft_node.h"
#include "butil/strings/string_split.h"

DEFINE_string(cluster, "http://127.0.0.1:9021", "comma separated cluster peers");
DEFINE_int32(id, 1, "node ID");
DEFINE_int32(kvport, 9121, "key-value server port");
DEFINE_bool(join, false, "join an existing cluster");


int main(int argc, char* argv[]) {
    // Parse gflags. We recommend you to use gflags as well.
	google::ParseCommandLineFlags(&argc, &argv, true);

    std::promise<anraft::ConfChange> promise_confchange;
    std::promise<std::string> promise_propose;
    std::promise<anraft::RaftError> promise_error;

    std::vector<std::string> peers;
    butil::SplitString(FLAGS_cluster, ',', &peers);
    example::RaftNode::Instance().Init(FLAGS_id, peers, FLAGS_join, NULL);
    example::RaftNode::Instance().Start();

    raftsnap::SnapshotterPtr snapshotter;
    //std::promise<std::string> promise_propose;
    std::promise<std::string> promise_commit;
    example::KvStore::NewKVStore(snapshotter, std::move(promise_propose), std::move(promise_commit));

    example::KvStorePtr kv_store;
    example::HttpApi::ServeHttpKVAPI(kv_store, FLAGS_kvport, std::move(promise_confchange), promise_error.get_future());
}
