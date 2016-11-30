//
// Copyright (c) 2016 Enrico Kaden & University College London
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef _FMT_H
#define _FMT_H

#include <string>

namespace smt {

int is_format_string(const std::string& fmt_str) {
	enum state_t {
		STATE_0,
		STATE_1,
		STATE_2
	};

	int ret = 0;
	state_t state = STATE_0;
	for(std::string::size_type ii = 0; ii < fmt_str.length(); ++ii) {
		if(fmt_str[ii] == '{') {
			if(state == STATE_0) {
				state = STATE_1;
			} else if(state == STATE_1) {
				state = STATE_0;
			} else if(state == STATE_2) {
				return -1;
			} else {
				return -1;
			}
		} else if(fmt_str[ii] == '}') {
			if(state == STATE_0) {
				state = STATE_2;
			} else if(state == STATE_1) {
				state = STATE_0;
				ret += 1;
			} else if(state == STATE_2) {
				state = STATE_0;
			} else {
				return -1;
			}
		} else {
			if(state == STATE_0) {
				;
			} else if(state == STATE_1) {
				return -1;
			} else if(state == STATE_2) {
				return -1;
			} else {
				return -1;
			}
		}
	}

	return (state == STATE_0)? ret : -1;
}

std::string format_string(const std::string& fmt_str, const std::string& arg = "") {
	enum state_t {
		STATE_0,
		STATE_1,
		STATE_2
	};

	std::string ret;
	state_t state = STATE_0;
	for(std::string::size_type ii = 0; ii < fmt_str.length(); ++ii) {
		if(fmt_str[ii] == '{') {
			if(state == STATE_0) {
				state = STATE_1;
			} else if(state == STATE_1) {
				state = STATE_0;
				ret += '{';
			} else if(state == STATE_2) {
				return std::string{};
			} else {
				return std::string{};
			}
		} else if(fmt_str[ii] == '}') {
			if(state == STATE_0) {
				state = STATE_2;
			} else if(state == STATE_1) {
				state = STATE_0;
				ret += arg;
			} else if(state == STATE_2) {
				state = STATE_0;
				ret += '}';
			} else {
				return std::string{};
			}
		} else {
			if(state == STATE_0) {
				ret += fmt_str[ii];
			} else if(state == STATE_1) {
				return std::string{};
			} else if(state == STATE_2) {
				return std::string{};
			} else {
				return std::string{};
			}
		}
	}

	return (state == STATE_0)? ret : std::string{};
}

} // smt

#endif // _FMT_H
