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

#include "raft_log.h"
#include "butil/logging.h"

namespace anraft {

static const std::string kCurrentTerm = "CURRENTTERM";

bool RaftLog::Open(const std::string& db_path) {
	// Create DB
	rocksdb::Options options;
	options.create_if_missing = true;
	options.write_buffer_size = 1024 * 1024 * 1024;
	options.max_background_flushes = 8;
	rocksdb::Status s = rocksdb::DB::Open(options, db_path, &log_);
	if (!s.ok()) {
		LOG(ERROR) << "Open db failed! path: " << db_path << ", status=" << s.ToString();
		return false;
	}


}

uint64_t RaftLog::GetCurrentTerm() {
	std::string buf;
	uint64_t ans;
	rocksdb::Status s = log_->Get(rocksdb::ReadOptions(), kCurrentTerm, &buf);
	if (s.IsNotFound()) {
		LOG(ERROR) << "GetCurrentTerm failed! status=" << s.ToString();
		return 0;
	}
	memcpy(&ans, buf.data(), sizeof(uint64_t));
	return ans;
}

void RaftLog::SetCurrentTerm(const uint64_t current_term) {
	char buf[8];
	memcpy(buf, &current_term, sizeof(uint64_t));
	log_->Put(rocksdb::WriteOptions(), kCurrentTerm, std::string(buf, 8));
	return;
}

bool RaftLog::ReadEntry(int64_t index, std::string* entry) {}
bool RaftLog::WriteEntry(int64_t index, const std::string& entry) {}

bool RaftLog::StoreMeta(const std::string& key, const std::string& value) {}
bool RaftLog::StoreMeta(const std::string& key, int64_t value) {}
bool RaftLog::GetMeta(const std::string& key, int64_t* value) {}
bool RaftLog::GetMeat(const std::string& key, std::string* value) {}

bool RaftLog::StoreLog(int64_t term, int64_t index, const std::string& log) {}
bool RaftLog::GetLog(int64_t term, int64_t index, std::string* log) {}

}