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

#include <iterator>
#include "storage.h"

namespace anraft {

std::tuple<HardState, ConfState, RaftError> MemoryStorage::InitialState() {
	return std::make_tuple(hard_state_, conf_state_, ErrNone);
}

std::tuple<std::vector<LogEntry>, RaftError> MemoryStorage::Entries(uint64_t lo, uint64_t hi, uint64_t max_size) {
	std::lock_guard<std::mutex> lock(mutex_);
	
	uint64_t offset = entries_[0].index();
	std::vector<LogEntry> target;
	if (lo <= offset) {
		return std::make_tuple(target, ErrCompacted);
	}
	if (hi > (offset + entries_.size())) {
		return std::make_tuple(target, ErrCompacted);
	}

	target.insert(target.begin(), entries_.begin() + offset, entries_.begin() + offset + hi - lo);
	//TODO: max_size check 
	return std::make_tuple(target, ErrNone);
}

std::tuple<uint64_t, RaftError> MemoryStorage::Term(uint64_t i) {
	std::lock_guard<std::mutex> lock(mutex_);

	uint64_t offset = entries_[0].index();
	if (i < offset) {
		return std::make_tuple(0, ErrCompacted);
	}

	if (i > (offset + entries_.size() - 1)) {
		return std::make_tuple(0, ErrUnavailable);
	}

	uint64_t term = entries_[i - offset].term();
	return std::make_tuple(term, ErrNone);
}

std::tuple<uint64_t, RaftError> MemoryStorage::LastIndex() {
	return std::make_tuple(1, ErrNone);
}

std::tuple<uint64_t, RaftError> MemoryStorage::FirstIndex() {
	return std::make_tuple(1, ErrNone);
}

std::tuple<Snapshot, RaftError> MemoryStorage::SnapShot() {
	return std::make_tuple(snap_shot_, ErrNone);  //TODO: Snapshot
}


MemoryStorage& MemoryStorage::NewMemoryStorage() {
	static MemoryStorage g_memory_storage_;
	return g_memory_storage_;
}

MemoryStorage::MemoryStorage() {
	// When starting from scratch populate the list with a dummy entry at term zero.
	entries_.push_back(LogEntry());
}

MemoryStorage::~MemoryStorage() {}

RaftError MemoryStorage::SetHardState(HardState hs) {
	std::lock_guard<std::mutex> lock(mutex_);
	hard_state_ = hs;
	return ErrNone;
}


RaftError MemoryStorage::Append(const std::vector<LogEntry> &entries) {
	if (entries.empty()) {
		return ErrNone;
	}

	std::lock_guard<std::mutex> lock(mutex_);

	uint64_t old_first = entries_[0].index() + 1;
	uint64_t old_last  = old_first + entries_.size() - 1;
	uint64_t new_first = entries[0].index();
	uint64_t new_last  = entries[0].index() + entries.size() - 1;

	// shortcut if there is no new entry.
	if (new_last <= old_first) {
		return ErrNone;
	}

	//missing log entry
	if (old_last < new_first) {
		return ErrNone;
	}

	uint64_t offset = old_last - new_first;
	entries_.reserve(entries_.size() + entries.size());
	std::move(entries.begin() + offset, entries.end(), std::back_inserter(entries_));
	return ErrNone;
}


RaftError MemoryStorage::Compact(uint64_t compact_index) {
	std::lock_guard<std::mutex> lock(mutex_);

	uint64_t offset = entries_[0].index();
	uint64_t last_index = offset + entries_.size() + 1;
	if (compact_index <= offset) {
		return ErrCompacted;
	}

	if (compact_index > last_index) {
		return ErrCompacted;
	}

	uint64_t compact_num = compact_index - offset;
	entries_[0].set_index(entries_[compact_num].index());
	entries_[0].set_term(entries_[compact_num].term());
	entries_.erase(entries_.begin() + 1, entries_.begin() + 1 + compact_num);
}


}
