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

#include "progress.h"

namespace anraft {

bool Progress::IsPaused() {
    switch (state_) {
    case ProgressStateProbe:
        return paused_;
    case ProgressStateReplicate:
        return false;//TODO: inflights
    case ProgressStateSnapshot:
        return true;
    }
}


bool Progress::MaybeUpdate(uint64_t n) {
    bool is_update = false;

    if (match_ < n) {
        match_ = n;
        is_update = true;
        paused_ = false;
    }

    if (next_ < n + 1) {
        next_ = n + 1;
    }

    return is_update;
}

} //namespace anraft