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

#ifndef SOURCE_DIRECTORY_NODE_H_
#define SOURCE_DIRECTORY_NODE_H_

#include <vector>
#include <future>
#include "config.h"
#include "raft.h"
#include "proto/raft.pb.h"
#include <bthread/execution_queue.h>


namespace anraft {

struct Peer {
    uint64_t id;
    std::string context;

    Peer(uint64_t p_id) : id(p_id) {}
};

// SoftState provides state that is useful for logging and debugging.
// The state is volatile and does not need to be persisted to the WAL.
struct SoftState {
    uint64_t lead;       // must use atomic operations to access; keep 64-bit aligned.
    StateType raft_state;
};

// Ready encapsulates the entries and messages that are ready to read,
// be saved to stable storage, committed or sent to other peers.
// All fields in Ready are read-only.
class Ready {
public:
    static Ready& NewReady(const Raft& raft, SoftState soft_state, HardState hard_state);
    bool ContainsUpdates();
private:
    // The current volatile state of a Node.
    // SoftState will be nil if there is no update.
    // It is not required to consume or store SoftState.
    //*SoftState

    // The current state of a Node to be saved to stable storage BEFORE
    // Messages are sent.
    // HardState will be equal to empty state if there is no update.
    //pb.HardState

    // ReadStates can be used for node to serve linearizable read requests locally
    // when its applied index is greater than the index in ReadState.
    // Note that the readState will be returned when raft receives msgReadIndex.
    // The returned is only valid for the request that requested to read.
    //ReadStates[]ReadState

    // Entries specifies entries to be saved to stable storage BEFORE
    // Messages are sent.
    //Entries[]pb.Entry

    // Snapshot specifies the snapshot to be saved to stable storage.
    //Snapshot pb.Snapshot

    // CommittedEntries specifies entries to be committed to a
    // store/state-machine. These have previously been committed to stable
    // store.
    //CommittedEntries[]pb.Entry

    // Messages specifies outbound messages to be sent AFTER Entries are
    // committed to stable storage.
    // If it contains a MsgSnap message, the application MUST report back to raft
    // when the snapshot has been received or has failed by calling ReportSnapshot.
    //Messages[]pb.Message

    // MustSync indicates whether the HardState and Entries must be synchronously
    // written to disk or if an asynchronous write is permissible.
    //MustSync bool

};

enum NodeRecvMsgType {
    MsgTypeProp = 0,
    MsgTypeReady
};

struct NodeRecvMsg {
    NodeRecvMsgType type;
    void* body;
};

class Node {
public:
    // StartNode returns a new Node given configuration and a list of raft peers.
    // It appends a ConfChangeAddNode entry for each given peer to the initial log.
    static Node& StartNode(const Config& config, const std::vector<Peer> &peers);

    // RestartNode is similar to StartNode but does not take a list of peers.
    // The current membership of the cluster will be restored from the Storage.
    // If the caller has an existing state machine, pass in the last log index that
    // has been applied to it; otherwise use zero.
    static Node& RestartNode(const Config& config);



    // Tick increments the internal logical clock for this Node. Election timeouts
    // and heartbeat timeouts are in units of ticks.
    void Tick();

    bthread::ExecutionQueueId<NodeRecvMsg> GetQueueID();
private:
    static int Run(void* meta, bthread::TaskIterator<NodeRecvMsg>& iter);


public:  //TODO to private
    Node();
    Node& operator=(const Node&);

private:
    std::future<Ready> future_ready_;
    bthread::ExecutionQueueId<NodeRecvMsg> queue_id_;

};


} //  namespace anraft

#endif //SOURCE_DIRECTORY_NODE_H_
