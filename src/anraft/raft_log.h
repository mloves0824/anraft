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

#ifndef SOURCE_DIRECTORY__SRC_ANRAFT_RAFT_LOG_H_
#define SOURCE_DIRECTORY__SRC_ANRAFT_RAFT_LOG_H_

#include "rocksdb/db.h"

namespace anraft {

class RaftLog {
public:
	RaftLog();
	virtual ~RaftLog();

	bool Open(const std::string& db_path);

	uint64_t GetCurrentTerm();
	void SetCurrentTerm(const uint64_t current_term);

	bool ReadEntry(int64_t index, std::string* entry);
	bool WriteEntry(int64_t index, const std::string& entry);

	bool StoreMeta(const std::string& key, const std::string& value);
	bool StoreMeta(const std::string& key, int64_t value);
	bool GetMeta(const std::string& key, int64_t* value);
	bool GetMeat(const std::string& key, std::string* value);

	bool StoreLog(int64_t term, int64_t index, const std::string& log);
	bool GetLog(int64_t term, int64_t index, std::string* log);
private:
	//log[] log entries; each entry contains command
	//	    for state machine, and term when entry
	//		was received by leader(first index is 1)	rocksdb::DB* log_;  
};
}















#endif