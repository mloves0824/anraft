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

#ifndef SOURCE_DIRECTORY_RAFTHTTP_TRANSPORT_H_
#define SOURCE_DIRECTORY_RAFTHTTP_TRANSPORT_H_

#include <memory> 
#include "proto/raft.pb.h"
#include "raftsnap/snapshotter.h"
#include "remote.h"
#include "peer.h"

namespace rafthttp {

class Transport;
typedef std::shared_ptr<Transport> TransportPtr;

//type Raft interface {
//    Process(ctx context.Context, m raftpb.Message) error
//        IsIDRemoved(id uint64) bool
//        ReportUnreachable(id uint64)
//        ReportSnapshot(id uint64, status raft.SnapshotStatus)
//}

struct Raft {
    virtual anraft::RaftError Process(anraft::Message) = 0;
};

// Transport implements Transporter interface. It provides the functionality
// to send raft messages to peers, and receive raft messages from peers.
// User should call Handler method to get a handler to serve requests
// received from peerURLs.
// User needs to call Start before calling other functions, and call
// Stop when the Transport is no longer used.
class Transport : public std::enable_shared_from_this<Transport> {
public:
    static Transport& Instance();
    bool Init(uint64_t id, 
              const std::vector<std::string>& urls, 
              uint64_t cluster_id,
              Raft *raft,
              raftsnap::SnapshotterPtr snapshotter);
    bool Start();
    PeerPtr StartPeer(TransportPtr transport,
					  const std::vector<std::string>& urls,
					  uint64_t peer_id);

    void AddPeer(uint64_t id, const std::vector<std::string>& urls);
    bool Send(const anraft::Message& msg);

private:

private:
    // local member ID
    uint64_t id_;
    // local peer URLs
    std::vector<std::string> urls_;
    // raft cluster ID for request validation
    uint64_t cluster_id_;
    // raft state machine, to which the Transport forwards received messages and reports status
    Raft *raft_;
    //snapshotter
    raftsnap::SnapshotterPtr snapshotter_;

    //mutex TODO         // protect the remote and peer map
    // remotes map that helps newly joined member to catch up
    std::map<uint64_t, RemotePtr> remotes_;
    // peers map
    std::map<uint64_t, PeerPtr> peers_;
    
    //dial timeout TODO
    // maximum duration before timing out dial of the request
    //DialTimeout time.Duration 
    // DialRetryFrequency defines the frequency of streamReader dial retrial attempts;
    // a distinct rate limiter is created per every peer (default value: 10 events/sec)
    //    DialRetryFrequency rate.Limit

    //Stats TODO
    //prober TODO
    //TLS TODO
};


} //namespace rafthttp

#endif //SOURCE_DIRECTORY_RAFTHTTP_TRANSPORT_H_
