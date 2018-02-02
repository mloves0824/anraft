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

#include <string>
#include "anraft_options.h"

namespace anraft {

enum Status {
    kOk = 0,
    kNotFound = 1,
    kCorruption = 2,
    kNotSupported = 3,
    kInvalidArgument = 4,
    kIOError = 5,
    kEndFile = 6,
    kIncomplete = 7,
    kComplete = 8,
    kTimeout = 9,
    kAuthFailed = 10
};

class AnRaft {
public:
	static Status Open(const AnraftOptions &options);

	AnRaft();
	virtual ~AnRaft();

	virtual Status Put(const std::string &key, const std::string &value);
	virtual Status Get(const std::string &key, std::string &value);

};

}
