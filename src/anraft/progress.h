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

#ifndef SOURCE_DIRECTORY_ANRAFT_PROGRESS_H_
#define SOURCE_DIRECTORY_ANRAFT_PROGRESS_H_

#include <stdint.h>

namespace anraft {

enum ProgressStateType {
    ProgressStateProbe = 0,
    ProgressStateReplicate,
    ProgressStateSnapshot
};

// Progress represents a follower��s progress in the view of the leader. Leader maintains
// progresses of all followers, and sends entries to the follower based on its progress.
class Progress {
public:
    // IsPaused returns whether sending log entries to this node has been
    // paused. A node may be paused because it has rejected recent
    // MsgApps, is currently waiting for a snapshot, or has reached the
    // MaxInflightMsgs limit.
    bool IsPaused();

    // maybeUpdate returns false if the given n index comes from an outdated message.
    // Otherwise it updates the progress and returns true.
    bool MaybeUpdate(uint64_t n);

    uint64_t Match() {return match_;}
    uint64_t Next() {return next_;}
    ProgressStateType State() {return state_;}

private:
    uint64_t match_;
    uint64_t next_;
    // State defines how the leader should interact with the follower.
    //
    // When in ProgressStateProbe, leader sends at most one replication message
    // per heartbeat interval. It also probes actual progress of the follower.
    //
    // When in ProgressStateReplicate, leader optimistically increases next
    // to the latest entry sent after sending replication message. This is
    // an optimized state for fast replicating log entries to the follower.
    //
    // When in ProgressStateSnapshot, leader should have sent out snapshot
    // before and stops sending any replication message.
    ProgressStateType state_;

    // Paused is used in ProgressStateProbe.
    // When Paused is true, raft should pause sending replication message to this peer.
    bool paused_;

};

} //namespace anraft

#endif //SOURCE_DIRECTORY_ANRAFT_PROGRESS_H_
