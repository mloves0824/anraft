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

#include <memory>
#include <bthread/execution_queue.h>
#include "proto/raft.pb.h"
#include "transport.h"

#ifndef SOURCE_DIRECTORY_RAFTHTTP_PEER_H_
#define SOURCE_DIRECTORY_RAFTHTTP_PEER_H_

namespace rafthttp {

// peer is the representative of a remote raft node. Local raft node sends
// messages to the remote through peer.
// Each peer has two underlying mechanisms to send out a message: stream and
// pipeline.
// A stream is a receiver initialized long-polling connection, which
// is always open to transfer messages. Besides general stream, peer also has
// a optimized stream for sending msgApp since msgApp accounts for large part
// of all messages. Only raft leader uses the optimized stream to send msgApp
// to the remote follower node.
// A pipeline is a series of http clients that send http requests to the remote.
// It is only used when the stream has not been established.
class Peer {
public:
    static PeerPtr StartPeer(TransportPtr transport,
                             const std::vector<std::string>& urls,
                             uint64_t peer_id);

    static int ServePropose(void* meta, bthread::TaskIterator<anraft::Message>& iter);
    static int ServeRecv(void* meta, bthread::TaskIterator<anraft::Message>& iter);


private:
    bthread::ExecutionQueueId<anraft::Message> recv_queue_id_;
    bthread::ExecutionQueueId<anraft::Message> prop_queue_id_;

};

typedef std::shared_ptr<Peer> PeerPtr;

} //namespace rafthttp

#endif //SOURCE_DIRECTORY_RAFTHTTP_PEER_H_
