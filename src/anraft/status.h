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

namespace anraft {

class Status {
public:
	//Create success status
	Status() : code_(kOk), subcode_(kNone) {}
	virtual ~Status() {}

	Status(const Status& s) : code_(s.code_), subcode_(s.subcode_) {}
	Status& operator=(const Status& s) {
		if (this != &s) {
			code_ = s.code_;
			subcode_ = s.subcode_;
		}
		return *this;
	}

	enum Code {
		kOk = 0,
		kInvalidArgument = 1,
		kNotSupported = 2,
		kMaxCode
	};

	enum SubCode {
		kNone = 0,
		kMaxSubCode
	};

private:
	Code code_;
	SubCode subcode_;
};

}