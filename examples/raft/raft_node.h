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
#include "proto/raft.pb.h"
#include "anraft/storage.h"
#include "anraft/node.h"

namespace example {
    
typedef std::function< std::tuple<std::vector<uint8_t>, anraft::RaftError>() > GetSnapshotFunc_t;

class RaftNode {
public:
	static RaftNode& NewRaftNode(int id, 
                                 const std::vector<std::string>& peers,
                                 bool join,
                                 GetSnapshotFunc_t getsnapshot_func);

private:
    RaftNode(int id,
             const std::vector<std::string>& peers,
             bool join,
             GetSnapshotFunc_t getsnapshot_func);

    void StartRaft();
    void ServeChannels();
    static void OnTickTimer(void *arg);

private:
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
	// index of log at start
	uint64_t last_index_;
	anraft::ConfState conf_state_;
	uint64_t snapshot_index_;
	uint64_t applied_index_;

	// raft backing for the commit/error channel
    anraft::Node node_;
	anraft::MemoryStorage& raft_storage_;
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

