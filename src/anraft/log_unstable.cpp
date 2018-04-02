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

int64_t Unstable::MaybeLastIndex() {}

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

} //namespace anraft
