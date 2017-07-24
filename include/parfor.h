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

#ifndef _PARFOR_H
#define _PARFOR_H

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>

#include "debug.h"
#include "env.h"

namespace smt {

unsigned int threads() {
	const std::string val_str{smt::getenv("SMT_NUM_THREADS")};

	if(val_str.empty()) {
		return std::thread::hardware_concurrency();
	} else {
		int val_int = std::atoi(val_str.c_str());

		if(val_int > 0) {
			return val_int;
		} else {
			smt::error("Unable to evaluate the environment variable ‘SMT_NUM_THREADS’.");
			std::exit(EXIT_FAILURE);

			return 0; // unreachable
		}
	}
}

template <typename Range, typename Func>
typename std::enable_if<Range::Dim == 1, void>::type parfor(const Range& rg, Func f,
		const unsigned int& nthreads = 1, const std::size_t& chunk = 1) {

	if(nthreads > 1) {
		std::vector<std::thread> threads;
		threads.reserve(nthreads);

		std::atomic<std::size_t> tmp{0};
		for(unsigned int ii = 0; ii < nthreads; ++ii) {
			threads.emplace_back([&](const unsigned int tt) {
				std::size_t jj;
				while((jj = tmp.fetch_add(chunk, std::memory_order_relaxed)) < rg.size()) {
					for(std::size_t kk = 0; kk < chunk && jj+kk < rg.size(); ++kk) {
						f(jj+kk, tt);
					}
				}
			}, ii);
		}

		for(std::thread &t : threads) {
			if(t.joinable()) {
				t.join();
			}
		}
	} else {
		for(std::size_t ii = 0; ii < rg.size(); ++ii) {
			f(ii, 0);
		}
	}
}

template <typename Range, typename Func>
typename std::enable_if<Range::Dim == 2, void>::type parfor(const Range& rg, Func f,
		const unsigned int& nthreads = 1, const std::size_t& chunk = 1) {

	if(nthreads > 1) {
		std::vector<std::thread> threads;
		threads.reserve(nthreads);

		std::atomic<std::size_t> tmp{0};
		for(unsigned int ii = 0; ii < nthreads; ++ii) {
			threads.emplace_back([&](const unsigned int tt) {
				std::size_t jj;
				while((jj = tmp.fetch_add(chunk, std::memory_order_relaxed)) < rg.size()) {
					for(std::size_t kk = 0; kk < chunk && jj+kk < rg.size(); ++kk) {
						std::size_t i0, i1;
						std::tie(i0, i1) = rg.index(jj+kk);
						f(i0, i1, tt);
					}
				}
			}, ii);
		}

		for(std::thread &t : threads) {
			if(t.joinable()) {
				t.join();
			}
		}
	} else {
		for(std::size_t ii = 0; ii < rg.size(); ++ii) {
			std::size_t i0, i1;
			std::tie(i0, i1) = rg.index(ii);
			f(i0, i1, 0);
		}
	}
}

template <typename Range, typename Func>
typename std::enable_if<Range::Dim == 3, void>::type parfor(const Range& rg, Func f,
		const unsigned int& nthreads = 1, const std::size_t& chunk = 1) {

	if(nthreads > 1) {
		std::vector<std::thread> threads;
		threads.reserve(nthreads);

		std::atomic<std::size_t> tmp{0};
		for(unsigned int ii = 0; ii < nthreads; ++ii) {
			threads.emplace_back([&](const unsigned int tt) {
				std::size_t jj;
				while((jj = tmp.fetch_add(chunk, std::memory_order_relaxed)) < rg.size()) {
					for(std::size_t kk = 0; kk < chunk && jj+kk < rg.size(); ++kk) {
						std::size_t i0, i1, i2;
						std::tie(i0, i1, i2) = rg.index(jj+kk);
						f(i0, i1, i2, tt);
					}
				}
			}, ii);
		}

		for(std::thread &t : threads) {
			if(t.joinable()) {
				t.join();
			}
		}
	} else {
		for(std::size_t ii = 0; ii < rg.size(); ++ii) {
			std::size_t i0, i1, i2;
			std::tie(i0, i1, i2) = rg.index(ii);
			f(i0, i1, i2, 0);
		}
	}
}

} // smt

#endif // _PARFOR_H
