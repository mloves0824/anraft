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
#ifndef SOURCE_DIRECTORY__SRC_ANRAFT_ANRAFT_OPTIONS_H_
#define SOURCE_DIRECTORY__SRC_ANRAFT_ANRAFT_OPTIONS_H_

#include <string>

namespace anraft {

struct AnraftOptions {
	AnraftOptions() : check_leader_us(0), heatbeat_us(0) {}
	virtual ~AnraftOptions() {}

	uint64_t check_leader_us;
	uint64_t heatbeat_us;
	std::vector<std::string> nodes;
	std::string local_addr;
	std::string db_path;
};

}


#endif /* SOURCE_DIRECTORY__SRC_ANRAFT_ANRAFT_OPTIONS_H_ */
