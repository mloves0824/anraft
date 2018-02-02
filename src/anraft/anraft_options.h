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

class AnraftOptions {
public:
	AnraftOptions();
	~AnraftOptions();

private:
	uint64_t check_leader_timer_;
	uint64_t heatbeat_timer_;
};

}


#endif /* SOURCE_DIRECTORY__SRC_ANRAFT_ANRAFT_OPTIONS_H_ */
