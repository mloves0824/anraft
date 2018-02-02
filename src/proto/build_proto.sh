#!/bin/bash


set -x

PROTOC=protoc
PROTOC_EXTRA_ARGS= 

$PROTOC --cpp_out=. --proto_path=. $PROTOC_EXTRA_ARGS ./raft.proto
$PROTOC --cpp_out=. --proto_path=. $PROTOC_EXTRA_ARGS ./raft_kv.proto
