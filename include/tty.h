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

#ifndef _TTY_H
#define _TTY_H

#include <cstdlib>
#include <ostream>
#include <string>

#include "env.h"

namespace smt {

namespace colour {

namespace {

bool colour() {
	const std::string val1{smt::getenv("SMT_NOCOLOR")};
	const std::string val2{smt::getenv("SMT_NOCOLOUR")};

	if(val1.empty()) {
		if(val2.empty()) {
			return true;
		} else {
			if(val2 == "true" || val2 == "True" || val2 == "TRUE" || std::atoi(val2.c_str()) > 0) {
				return false;
			} else {
				return true;
			}
		}
	} else {
		if(val2.empty()) {
			if(val1 == "true" || val1 == "True" || val1 == "TRUE" || std::atoi(val1.c_str()) > 0) {
				return false;
			} else {
				return true;
			}
		} else {
			if((val1 == "true" || val1 == "True" || val1 == "TRUE" || std::atoi(val1.c_str()) > 0)
					&& (val2 == "true" || val2 == "True" || val2 == "TRUE" || std::atoi(val2.c_str()) > 0)) {
				return false;
			} else {
				return true;
			}
		}
	}
}

} // (anonymous)

std::ostream& reset(std::ostream& s) {
	if(colour()) {
		s << "\033[0m";
	}
	return s;
}

std::ostream& bold(std::ostream& s) {
	if(colour()) {
		s << "\033[1m";
	}
	return s;
}

std::ostream& black(std::ostream& s) {
	if(colour()) {
		s << "\033[30m";
	}
	return s;
}

std::ostream& red(std::ostream& s) {
	if(colour()) {
		s << "\033[31m";
	}
	return s;
}

std::ostream& green(std::ostream& s) {
	if(colour()) {
		s << "\033[32m";
	}
	return s;
}

std::ostream& yellow(std::ostream& s) {
	if(colour()) {
		s << "\033[33m";
	}
	return s;
}

std::ostream& blue(std::ostream& s) {
	if(colour()) {
		s << "\033[34m";
	}
	return s;
}

std::ostream& magenta(std::ostream& s) {
	if(colour()) {
		s << "\033[35m";
	}
	return s;
}

std::ostream& cyan(std::ostream& s) {
	if(colour()) {
		s << "\033[36m";
	}
	return s;
}

std::ostream& grey(std::ostream& s) {
	if(colour()) {
		s << "\033[37m";
	}
	return s;
}

} //colour

} // smt

#endif // _TTY_H
