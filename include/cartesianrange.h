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

#ifndef _CARTESIANRANGE_H
#define _CARTESIANRANGE_H

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace smt {

template <unsigned int D>
class cartesianrange {
public:
	static const unsigned int Dim = D;

	cartesianrange(const std::size_t& s0) {
		static_assert(D == 1, "D == 1");
		_size[0] = s0;
	}

	cartesianrange(const std::size_t& s0, const std::size_t& s1) {
		static_assert(D == 2, "D == 2");
		_size[0] = s0;
		_size[1] = s1;
	}

	cartesianrange(const std::size_t& s0, const std::size_t& s1, const std::size_t& s2) {
		static_assert(D == 3, "D == 3");
		_size[0] = s0;
		_size[1] = s1;
		_size[2] = s2;
	}

	std::size_t size() const {
		std::size_t total_size = 1;
		for(unsigned int ii = 0; ii < D; ++ii) {
			total_size *= _size[ii];
		}
		return total_size;
	}

	template <unsigned int E = D>
	typename std::enable_if<E == 1, std::size_t>::type index(const std::size_t& ii) const {
		return ii;
	}

	template <unsigned int E = D>
	typename std::enable_if<E == 2, std::tuple<std::size_t, std::size_t>>::type index(const std::size_t& ii) const {
		return std::make_tuple(ii/_size[1], ii%_size[1]);
	}

	template <unsigned int E = D>
	typename std::enable_if<E == 3, std::tuple<std::size_t, std::size_t, std::size_t>>::type index(const std::size_t& ii) const {
		return std::make_tuple(ii/(_size[1]*_size[2]), (ii/_size[2])%_size[1], ii%_size[2]);
	}

	~cartesianrange() {
	}
private:
	std::size_t _size[D];
};

} // smt

#endif // _CARTESIANRANGE_H
