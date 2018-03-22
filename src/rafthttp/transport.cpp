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

#include "transport.h"
#include "stream.h"

namespace rafthttp {

Transport& Transport::Instance() {
    static Transport g_transport;
    return g_transport;
}

bool Transport::Init(uint64_t id,
                     const std::vector<std::string>& urls,
                     uint64_t cluster_id,
                     Raft *raft,
                     raftsnap::SnapshotterPtr snapshotter) {
    id_ = id;
    urls_ = urls;
    cluster_id_ = cluster_id;
    raft_ = raft;
    snapshotter_ = snapshotter;
}

PeerPtr Transport::StartPeer(TransportPtr transport,
				             const std::vector<std::string>& urls,
				             uint64_t peer_id) {
    //TODO chenbang: pipeline.start()

    //create peer ant init
    PeerPtr peer(new Peer());

    //start bthread for recv_queue_id_
    bthread::execution_queue_start(peer->GetRecvQueue(),
                                    NULL,
                                    Peer::ServeRecv,
                                    (void*)peer.get());

    //start bthread for prop_queue_id_
    // r.Process might block for processing proposal when there is no leader.
    // Thus propc must be put into a separate routine with recvc to avoid blocking
    // processing other raft messages.
    bthread::execution_queue_start(peer->GetPropQueue(),
                                    NULL,
                                    Peer::ServePropose,
                                    (void*)peer.get());

    //TODO chenbang: p.msgAppReader.start()
    StreamWriter::Start(urls[peer_id-1]);
    StreamReader::Start(1000);//TODO
}

bool Transport::Start() {}

void Transport::AddPeer(uint64_t id, const std::vector<std::string>& urls) {
    
    peers_[id] = StartPeer(shared_from_this(), urls, id);
}

bool Transport::Send(const anraft::Message& msg) {}


} //namespace rafthttp
