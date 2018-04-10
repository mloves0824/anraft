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
	auto mli_ret = unstable_.MaybeLastIndex();
	if (std::get<1>(mli_ret))
		return std::get<0>(mli_ret);

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

uint64_t RaftLog::Append(::google::protobuf::RepeatedPtrField<::anraft::LogEntry>& entries) {}

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

    //if (unstable_) //TODO
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
    //find unstable firstly
    auto mfi_ret = unstable_.MaybeFirstIndex();
    if (std::get<1>(mfi_ret)) {
    	return std::get<0>(mfi_ret);
    }

    auto fi_ret = storage_->FirstIndex();
    if (std::get<1>(fi_ret) != ErrNone) {
    	//TODO: panic
    }

    return std::get<0>(fi_ret);
}


// l.firstIndex <= lo <= hi <= l.firstIndex + len(l.entries)
RaftError RaftLog::MustCheckOutOfBounds(uint64_t lo, uint64_t hi) {
	if (lo > hi) {
		return ErrParam;
	}

	auto fi = FirstIndex();
	if (lo < fi)
		return ErrCompacted;

	auto length = LastIndex() + 1 - fi;
	if (lo < fi || hi > fi + length) //TODO: why lo < fi is here ?
		return ErrCompacted;

	return ErrNone;
}


// slice returns a slice of log entries from lo through hi-1, inclusive.
//func (l *raftLog) slice(lo, hi, maxSize uint64) ([]pb.Entry, error) {
RaftError RaftLog::Slice(uint64_t lo, uint64_t hi, uint64_t max_size, std::vector<LogEntry>& entries) {
	auto ret = MustCheckOutOfBounds(lo, hi);
	if (ret != ErrNone) {
		return ret;
	}

	if (lo == hi)
		return ErrNone;

	if (lo < unstable_.Offset()) {
		auto e_ret = storage_->Entries(lo, std::min(hi, unstable_.Offset()), max_size);
		if (std::get<1>(e_ret) != ErrNone) {
			return std::get<1>(e_ret);
		}

		//TODO:check if ents has reached the size limitation
		entries.insert(entries.end(), std::get<0>(e_ret).begin(), std::get<0>(e_ret).end());

	}

	if (hi > unstable_.Offset()) {

		//unstable := l.unstable.slice(max(lo, l.unstable.offset), hi)
		std::vector<LogEntry> unstable_entries;
		unstable_.Slice(std::max(lo, unstable_.Offset()), hi, unstable_entries);
		if (!unstable_entries.empty())
			entries.insert(entries.end(), unstable_entries.begin(), unstable_entries.end());

	}

	//TODO :limitSize(ents, maxSize)
	return ErrNone;

}



RaftError RaftLog::Entries(uint64_t index, uint64_t max_size, std::vector<LogEntry>& entries) {

	if (index > LastIndex())
		return ErrNone;

	return Slice(index, LastIndex() + 1, max_size, entries);
}

//// allEntries returns all entries in the log.
//func (l *raftLog) allEntries() []pb.Entry {
//	ents, err := l.entries(l.firstIndex(), noLimit)
//	if err == nil {
//		return ents
//	}
//	if err == ErrCompacted { // try again if there was a racing compaction
//		return l.allEntries()
//	}
//	// TODO (xiangli): handle error?
//	panic(err)
//}

RaftError RaftLog::AllEntries(std::vector<LogEntry>& entries) {
	return Entries(FirstIndex(), 10000, entries); //TODO: noLimit
	//TODO: try again if there was a racing compaction
}


uint64_t RaftLog::ZeroTermOnErrCompacted(std::tuple<anraft::RaftError, uint64_t> result) {

	RaftError error;
	if ((error = std::get<0>(result)) == ErrNone) {
		return std::get<1>(result);
	}

	//如果发生错误,则term为0
	if (error == ErrCompacted)
		return 0;

	//TODO panic
	//	l.logger.Panicf("unexpected error (%v)", err)
	return 0;
}

bool RaftLog::MaybeCommit(uint64_t index, uint64_t term) {
	if (index > committed_ && ZeroTermOnErrCompacted(Term(index)) == term) {
		CommitTo(index);
		return true;
	}

	return false;
}

void RaftLog::CommitTo(uint64_t tocommit) {
	//never decrease commit
	if (committed_ < tocommit) {
		if (LastIndex() < tocommit) {
			//TODO :panic
			//l.logger.Panicf("tocommit(%d) is out of range [lastIndex(%d)]. Was the raft log corrupted, truncated, or lost?", tocommit, l.lastIndex())
		}
		committed_ = tocommit;
	}
}

bool RaftLog::MatchTerm(uint64_t index, uint64_t term) {
    auto term_result = Term(index);
    if (std::get<0>(term_result) != ErrNone)
        return false;
    return std::get<1>(term_result) == term;
}

// findConflict finds the index of the conflict.
// It returns the first pair of conflicting entries between the existing
// entries and the given entries, if there are any.
// If there is no conflicting entries, and the existing entries contains
// all the given entries, zero will be returned.
// If there is no conflicting entries, but the given entries contains new
// entries, the index of the first new entry will be returned.
// An entry is considered to be conflicting if it has the same index but
// a different term.
// The first entry MUST have an index equal to the argument 'from'.
// The index of the given entries MUST be continuously increasing.
uint64_t RaftLog::FindConflict(PbVectorLogentryType& entries) {
    for (auto &x : entries) {
        if (!MatchTerm(x.index(), x.term())) {
            if (x.index() <= LastIndex()) {
                //TODO: 
                //l.logger.Infof("found conflict at index %d [existing term: %d, conflicting term: %d]",
                //ne.Index, l.zeroTermOnErrCompacted(l.term(ne.Index)), ne.Term)
            }
            return x.index();
        }
    }
    return 0;
}


}
