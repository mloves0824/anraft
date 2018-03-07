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
#ifndef SOURCE_DIRECTORY_STORAGE_H_
#define SOURCE_DIRECTORY_STORAGE_H_

#include <tuple>
#include <vector>
#include <cinttypes>
#include <mutex>
#include "raft.pb.h"

namespace anraft {

// Storage is an interface that may be implemented by the application
// to retrieve log entries from storage.
//
// If any Storage method returns an error, the raft instance will
// become inoperable and refuse to participate in elections; the
// application is responsible for cleanup and recovery in this case.
class Storage {
public:
	// InitialState returns the saved HardState and ConfState information.
	virtual std::tuple<HardState, ConfState, RaftError> InitialState() = 0;

	// Entries returns a slice of log entries in the range [lo,hi).
	// MaxSize limits the total size of the log entries returned, but
	// Entries returns at least one entry if any.
	virtual std::tuple<std::vector<LogEntry>, RaftError> Entries(uint64_t lo, uint64_t hi, uint64_t max_size) = 0;

	// Term returns the term of entry i, which must be in the range
	// [FirstIndex()-1, LastIndex()]. The term of the entry before
	// FirstIndex is retained for matching purposes even though the
	// rest of that entry may not be available.
	virtual std::tuple<uint64_t, RaftError> Term(uint64_t i) = 0;

	// LastIndex returns the index of the last entry in the log.
	//LastIndex() (uint64, error)
	virtual std::tuple<uint64_t, RaftError> LastIndex() = 0;

	// FirstIndex returns the index of the first log entry that is
	// possibly available via Entries (older entries have been incorporated
	// into the latest Snapshot; if storage only contains the dummy entry the
	// first log entry is not available).
	virtual std::tuple<uint64_t, RaftError> FirstIndex() = 0;

	// Snapshot returns the most recent snapshot.
	// If snapshot is temporarily unavailable, it should return ErrSnapshotTemporarilyUnavailable,
	// so raft state machine could know that Storage needs some time to prepare
	// snapshot and call Snapshot later.
	virtual std::tuple<Snapshot, RaftError> SnapShot() = 0;
};

class MemoryStorage final: Storage {
public:
	virtual std::tuple<HardState, ConfState, RaftError> InitialState();
	virtual std::tuple<std::vector<LogEntry>, RaftError> Entries(uint64_t lo, uint64_t hi, uint64_t max_size);
	virtual std::tuple<uint64_t, RaftError> Term(uint64_t i);
	virtual std::tuple<uint64_t, RaftError> LastIndex();
	virtual std::tuple<uint64_t, RaftError> FirstIndex();
	virtual std::tuple<Snapshot, RaftError> SnapShot();

	// SetHardState saves the current HardState.
	RaftError SetHardState(HardState hs);
	// Append the new entries to storage.
	RaftError Append(const std::vector<LogEntry> &entries);
	// Compact discards all log entries prior to compactIndex.
	// It is the application's responsibility to not attempt to compact an index
	// greater than raftLog.applied.
	RaftError Compact(uint64_t compact_index);
public:
	// NewMemoryStorage creates an empty MemoryStorage.
	static MemoryStorage& NewMemoryStorage();
	MemoryStorage();
	~MemoryStorage();

private:
	HardState hard_state_;
	ConfState conf_state_;
	Snapshot snap_shot_;
	std::vector<LogEntry> entries_;
	std::mutex mutex_;
};

}

#endif