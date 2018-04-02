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

#ifndef SOURCE_DIRECTORY_RAFT_LOG_UNSTABLE_H_
#define SOURCE_DIRECTORY_RAFT_LOG_UNSTABLE_H_

#include <vector>
#include <stdint.h>
#include <tuple>
#include "proto/raft.pb.h"

namespace anraft {

// unstable.entries[i] has raft log position i+unstable.offset.
// Note that unstable.offset may be less than the highest log
// position in storage; this means that the next write to storage
// might need to truncate the log before persisting unstable.entries.
class Unstable {
public:
    void SetOffset(uint64_t offset) { offset_ = offset; }
    // maybeLastIndex returns the last index if it has at least one
    // unstable entry or snapshot.
    //func(u *unstable) maybeLastIndex() (uint64, bool)
    // < 0 means error
    std::tuple<uint64_t, bool> MaybeLastIndex();

    //func(u *unstable) truncateAndAppend(ents[]pb.Entry) {
    void TruncateAndAppend(std::vector<LogEntry> entries);

    // maybeFirstIndex returns the index of the first possible entry in entries
    // if it has a snapshot.
    std::tuple<uint64_t, bool> MaybeFirstIndex();
    uint64_t Offset() {return offset_;}
    void Slice(uint64_t lo, uint64_t hi, std::vector<LogEntry>& entries);

private:
    // the incoming unstable snapshot, if any.
    Snapshot snapshot_;
    // all entries that have not yet been written to storage.
    std::vector<LogEntry> entries_;
    uint64_t offset_;
};

} //namespace anraft

#endif //SOURCE_DIRECTORY_RAFT_LOG_UNSTABLE_H_
