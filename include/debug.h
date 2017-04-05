//
// Copyright (c) 2016-2017 Enrico Kaden & University College London
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

#ifndef _DEBUG_H
#define _DEBUG_H

#include <cstdlib>
#include <iostream>
#include <string>

#include "env.h"
#include "tty.h"

namespace smt {

namespace {

bool debug() {
	const std::string val{smt::getenv("SMT_DEBUG")};
	if(val == "true" || val == "True" || val == "TRUE" || std::atoi(val.c_str()) > 0) {
		return true;
	} else {
		return false;
	}
}

} // (anonymous)

#ifdef NDEBUG
void assert_noimpl() {}

#define assert(test) assert_noimpl()
#else
void assert_impl(const bool& test, const std::string& s, const std::string& file, const long int& line, const std::string& function) {
	if(! test) {
#ifdef mex_h
		mexPrintAssertion(s.c_str(), file.c_str(), line.c_str(), nullptr);
#else
		std::cerr << smt::colour::red << "*** ERROR: " << file << ":" << line << ": In function ‘" << function << "’: Assertion ‘" << s << "’ failed." << smt::colour::reset << std::endl;
		std::exit(EXIT_FAILURE);
#endif // mex_h
	}
}

#define assert(test) assert_impl((test), #test, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif // NDEBUG

} // smt

#endif // _DEBUG_H
