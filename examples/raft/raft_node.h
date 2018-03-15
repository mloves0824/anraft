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

#ifndef SOURCE_DIRECTORY__SRC_EXAMPLES_RAFT__H_
#define SOURCE_DIRECTORY__SRC_EXAMPLES_RAFT__H_

#include <string>
#include <vector>
#include <stdint.h>
#include <functional>
#include <tuple>
#include <future>
#include "proto/raft.pb.h"
#include "anraft/storage.h"
#include "anraft/node.h"
#include "raftsnap/snapshotter.h"
#include "wal/wal.h"
#include "rafthttp/transport.h"

namespace example {
    
typedef std::function< std::tuple<std::vector<uint8_t>, anraft::RaftError>() > GetSnapshotFunc_t; //TODO
typedef std::tuple<std::promise<std::string>, std::promise<anraft::RaftError>,  std::promise<raftsnap::SnapshotterPtr> > NewRaftNodeReturnType;

const uint64_t default_snap_count = 10000;


class RaftNode {
public:
    static NewRaftNodeReturnType NewRaftNode(int id,
                                            const std::vector<std::string>& peers,
                                            bool join,
                                            GetSnapshotFunc_t getsnapshot_func,
                                            std::promise<std::string> promise_propose,
                                            std::promise<anraft::ConfChange> promise_confchange);

private:
    RaftNode(int id,
             const std::vector<std::string>& peers,
             bool join,
             GetSnapshotFunc_t getsnapshot_func,
             std::promise<std::string> promise_propose,
             std::promise<anraft::ConfChange> promise_confchange);

    void StartRaft();
    void ServeChannels();
    static void OnTickTimer(void *arg);

private:
    std::promise<std::string> promise_propose_;
    std::promise<anraft::ConfChange> promise_confchange;
    std::promise<std::string> promise_commit_;
    std::promise<anraft::RaftError> promise_error_;

	// client ID for raft session
    int id_;
	// raft peer URLs
	std::vector<std::string> peers_;
	// node is joining an existing cluster
	bool join_;
	// path to WAL directory
	std::string waldir_;
	// path to snapshot directory
	std::string snapdir_;
    GetSnapshotFunc_t get_snapshot_func_;
	// index of log at start
	uint64_t last_index_;
	anraft::ConfState conf_state_;
	uint64_t snapshot_index_;
	uint64_t applied_index_;

	// raft backing for the commit/error channel
    anraft::Node node_;
	anraft::MemoryStorage& raft_storage_;
    wal::WalPtr  wal_;
    raftsnap::SnapshotterPtr snapshotter_;
    std::promise<raftsnap::SnapshotterPtr> snapshotter_ready_;

    uint64_t snap_count_;
    rafthttp::TransportPtr transport_;
    std::promise<bool> stopc_;
    std::promise<bool> httpstopc_;
    std::promise<bool> httpdonec_;
};

}

#endif


/*

func newRaftNode(id int, peers []string, join bool, getSnapshot func() ([]byte, error), proposeC <-chan string,
confChangeC <-chan raftpb.ConfChange) (<-chan *string, <-chan error, <-chan *raftsnap.Snapshotter) {


// A key-value stream backed by raft
type raftNode struct {
	proposeC    <-chan string            // proposed messages (k,v)
		confChangeC <-chan raftpb.ConfChange // proposed cluster config changes
		commitC     chan<-*string           // entries committed to log (k,v)
		errorC      chan<-error             // errors from raft session

		id          int      // client ID for raft session
		peers[]string // raft peer URLs
		join        bool     // node is joining an existing cluster
		waldir      string   // path to WAL directory
		snapdir     string   // path to snapshot directory
		getSnapshot func() ([]byte, error)
		lastIndex   uint64 // index of log at start

		confState     raftpb.ConfState
		snapshotIndex uint64
		appliedIndex  uint64

		// raft backing for the commit/error channel
		node        raft.Node
		raftStorage *raft.MemoryStorage
		wal         *wal.WAL

		snapshotter      *raftsnap.Snapshotter
		snapshotterReady chan *raftsnap.Snapshotter // signals when snapshotter is ready

		snapCount uint64
		transport *rafthttp.Transport
		stopc     chan struct{} // signals proposal channel closed
	httpstopc chan struct{} // signals http server to shutdown
	httpdonec chan struct{} // signals http server shutdown complete
}
*/

