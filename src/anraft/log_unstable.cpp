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

#include "log_unstable.h"

namespace anraft {

// maybeLastIndex returns the last index if it has at least one
// unstable entry or snapshot.
//func (u *unstable) maybeLastIndex() (uint64, bool) {
//	if l := len(u.entries); l != 0 {
//		return u.offset + uint64(l) - 1, true
//	}
//	if u.snapshot != nil {
//		return u.snapshot.Metadata.Index, true
//	}
//	return 0, false
//}

std::tuple<uint64_t, bool> Unstable::MaybeLastIndex() {
	if(!entries_.empty()) { //TODO: why offset_ + entries_.size() - 1 ???
		return std::make_tuple<uint64_t, bool>(offset_ + entries_.size() - 1, true);
	}

	//TODO: snapshot

	return std::make_tuple<uint64_t, bool>(0, false);
}

void Unstable::TruncateAndAppend(std::vector<LogEntry> entries) {
    if (entries.empty())
        return;
    uint64_t after = entries[0].index();

    if (after == (offset_ + entries_.size())) {
        // after is the next index in the u.entries
        // directly append
        entries_.insert(entries_.end(), entries.begin(), entries.end());
    }
    else if (after <= offset_) {
        //ODO: u.logger.Infof("replace the unstable entries from index %d", after)
            // The log is being truncated to before our current offset
            // portion, so set the offset and replace the entries
        offset_ = after;
        entries_ = entries;
    }
    else {
        // truncate to after and copy to u.entries
        // then append
        //u.logger.Infof("truncate the unstable entries before index %d", after)
        std::vector<LogEntry> temp;
        temp.insert(temp.begin(), entries_.begin() + offset_, entries_.begin()+after);
        entries_.swap(temp);
    }
}


// maybeFirstIndex returns the index of the first possible entry in entries
// if it has a snapshot.
//func (u *unstable) maybeFirstIndex() (uint64, bool) {
//	if u.snapshot != nil {
//		return u.snapshot.Metadata.Index + 1, true
//	}
//	return 0, false
//}

std::tuple<uint64_t, bool> Unstable::MaybeFirstIndex() {

	//TODO: snapshot  why snapshot? what's meaning of the function?

	return std::make_tuple<uint64_t, bool>(0, false);
}


//func (u *unstable) slice(lo uint64, hi uint64) []pb.Entry {
//	u.mustCheckOutOfBounds(lo, hi)
//	return u.entries[lo-u.offset : hi-u.offset]
//}

void Unstable::Slice(uint64_t lo, uint64_t hi, std::vector<LogEntry>& entries) {
	//TODO: mustCheckOutOfBounds
	entries.insert(entries.begin(), entries_.begin() + lo - offset_, entries_.begin() + hi - offset_);
}

//// u.offset <= lo <= hi <= u.offset+len(u.offset)
//func (u *unstable) mustCheckOutOfBounds(lo, hi uint64) {



} //namespace anraft
