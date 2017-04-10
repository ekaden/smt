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

#ifndef _OPTS_H
#define _OPTS_H

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "docopt.h"

#include "debug.h"

namespace smt {

std::map<std::string, docopt::value> docopt(
		std::string const& doc,
		std::vector<std::string> const& argv,
		bool help = true,
		std::string const& version = {},
		bool options_first = false) {

	try {
		return docopt::docopt_parse(doc, argv, help, !version.empty(), options_first);
	} catch(docopt::DocoptExitHelp const&) {
		std::cout << doc << std::endl;
		std::exit(EXIT_SUCCESS);
	} catch(docopt::DocoptExitVersion const&) {
		std::cout << version << std::endl;
		std::exit(EXIT_SUCCESS);
	} catch(docopt::DocoptLanguageError const& error) {
		smt::error(std::string{"Docopt usage string could not be parsed. "} + error.what() + ".");
		std::exit(EXIT_FAILURE);
	} catch(docopt::DocoptArgumentError const& error) {
		smt::error(std::string{""} + error.what() + ".");
		std::cout << doc << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

} // smt

#endif // _OPTS_H
