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

RaftLog& RaftLog::GetRaftLog() {
    static RaftLog g_raft_log;
    return g_raft_log;
}

RaftLog::RaftLog() {}
RaftLog::~RaftLog() {}

bool RaftLog::NewLog(Storage *storage) {
    if (!storage)
        return false;

    storage_ = storage;

    auto fi_ret = storage_->FirstIndex();
    if (std::get<1>(fi_ret) != ErrNone)
        return false;
    auto first_index = std::get<0>(fi_ret);

    auto li_ret = storage_->LastIndex();
    if (std::get<1>(li_ret) != ErrNone)
        return false;
    auto last_index = std::get<0>(li_ret);

    unstable_.SetOffset(last_index + 1);
    // Initialize our committed and applied pointers to the time of the last compaction.
    committed_ = first_index - 1;
    applied_ = first_index - 1;
}

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

uint64_t RaftLog::LastIndex() {
    int64_t i = unstable_.MaybeLastIndex();
    if (i >= 0)
        return (uint64_t)i;

    auto li_ret = storage_->LastIndex();
    if (std::get<1>(li_ret) != ErrNone) {
        //TODO: panic
    }
    return std::get<0>(li_ret);
}

uint64_t RaftLog::LastTerm() {}
uint64_t RaftLog::Append(const LogEntry& log_entry) {}

uint64_t RaftLog::Append(const std::vector<LogEntry>& entries) {
    if (entries.empty())
        return LastIndex();

    auto after = entries[0].index() - 1;
    if (after < committed_) {
        //TODO: l.logger.Panicf("after(%d) is out of range [committed(%d)]", after, l.committed)
    }
    unstable_.TruncateAndAppend(entries);
    return LastIndex();
}

void RaftLog::SetCommited(uint64_t commited) { committed_ = commited; }

std::tuple<anraft::RaftError, anraft::LogEntry> RaftLog::Slice(uint64_t lo, uint64_t hi, uint64_t max_size) {

}

// isUpToDate determines if the given (lastIndex,term) log is more up-to-date
// by comparing the index and term of the last entries in the existing logs.
// If the logs have last entries with different terms, then the log with the
// later term is more up-to-date. If the logs end with the same term, then
// whichever log has the larger lastIndex is more up-to-date. If the logs are
// the same, the given log is up-to-date.
//func (l *raftLog) isUpToDate(lasti, term uint64) bool {
//	return term > l.lastTerm() || (term == l.lastTerm() && lasti >= l.lastIndex())
//}
bool RaftLog::IsUpToDate(uint64_t last_index, uint64_t term) {
	return term > this->LastTerm() || (term == this->LastTerm() && last_index > this->LastIndex());
}


//func(l *raftLog) term(i uint64) (uint64, error) {
//    // the valid term range is [index of dummy entry, last index]
//dummyIndex: = l.firstIndex() - 1
//    if i < dummyIndex || i > l.lastIndex() {
//        // TODO: return an error instead?
//        return 0, nil
//    }
//
//            if t, ok : = l.unstable.maybeTerm(i); ok{
//                return t, nil
//            }
//
//            t, err : = l.storage.Term(i)
//            if err == nil{
//                return t, nil
//            }
//            if err == ErrCompacted || err == ErrUnavailable{
//                return 0, err
//            }
//            panic(err) // TODO(bdarnell)
//}

std::tuple<anraft::RaftError, uint64_t> RaftLog::Term(uint64_t index) {
    // the valid term range is [index of dummy entry, last index]
    uint64_t dummy_index = FirstIndex() - 1;
    if (index < dummy_index || index > LastIndex()) {
        //        // TODO: return an error instead?
        //        return 0, nil
        return std::make_tuple<RaftError, uint64_t>(ErrNone, 0);
    }

    //if (unstable_)
}

//func(l *raftLog) firstIndex() uint64{
//    if i, ok : = l.unstable.maybeFirstIndex(); ok{
//        return i
//    }
//    index, err : = l.storage.FirstIndex()
//    if err != nil{
//        panic(err) // TODO(bdarnell)
//    }
//    return index
//}

uint64_t RaftLog::FirstIndex() {
    //if ()
}



}
