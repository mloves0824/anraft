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

#include "kvstore.h"
#include "raft_node.h"

namespace example {

KvStore& KvStore::Instance() {
	static KvStore g_kvstore;
	return g_kvstore;
}

bool KvStore::NewKVStore(raftsnap::SnapshotterPtr snapshotter) {
	snapshotter_ = snapshotter;
	if (bthread::execution_queue_start(&queue_id_,
		NULL,
		KvStore::ReadCommits,
		this)) {
		//TODO chenb: log
		return false;
		}
	}
}

void KvStore::Propose(const std::string& key, const std::string& value) {
    KV kv;
    kv.set_key(key);
    kv.set_val(value);
    std::string buf;
    kv.SerializeToString(&buf);

    RaftNode::Instance().Propose(buf);
}

bool KvStore::Commit(const KvStoreChannalMsg& msg) {
	if (bthread::execution_queue_execute(queue_id_, msg)) {
		//TODO chenb: log
		return false;
	}
	return true;
}

bool KvStore::Lookup(const std::string& key, const std::string& value) {
	std::lock_guard<std::mutex> guard(mutex_);
	value = kv_store_[key];
	return true;
}


int KvStore::ReadCommits(void* meta, bthread::TaskIterator<KvStoreChannalMsg>& iter) {
	KvStore* kv = (KvStore*)meta;
    if (iter.is_queue_stopped()) {
        //kv->do_shutdown(); //TODO
        return 0;
    }

    for (; iter; ++iter) {
		switch (iter->cmd) {
		case MsgTypeKV:
			if (iter->body.empty()) {}

			KV kv;
			if (!kv.ParseFromString(iter->body)) {}   //TODO
				//kv->kv_store_[kv.key()] = kv.val();
			break;

		case MsgTypeError:
			break;

		default:
			break;
		}


    }

}

} //namespace example
