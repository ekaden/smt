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

#ifndef _SLICABLE_H
#define _SLICABLE_H

#include <cstddef>
#include <type_traits>

#include "expression.h"
#include "indexable.h"

namespace smt {

// TODO: unsigned int => std::size_t, signed int => std::ptrdiff_t

class slice {
public:
	slice(unsigned int start, unsigned int size, int stride = 1):
			_start(start), _size(size), _stride(stride) {}

	unsigned int start() const {
		return _start;
	}

	unsigned int size() const {
		return _size;
	}

	int stride() const {
		return _stride;
	}

private:
	const unsigned int _start;
	const unsigned int _size;
	const int _stride;
};

template <typename C>
class slice_indexable : public slice {
public:
	typedef typename C::value_type value_type;

	explicit slice_indexable(const slice& s, C& a): slice(s), _a(a) {}

	template <typename RHS>
	slice_indexable<C>& operator=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(unsigned int ii = 0; ii < size(); ++ii) {
			operator[](ii) = ExprT(rhs)[ii];
		}

		return *this;
	}

	value_type& operator[](const unsigned int& ii) {
		return _a[start()+ii*stride()];
	}

	const value_type& operator[](const unsigned int& ii) const {
		return _a[start()+ii*stride()];
	}

private:
	C& _a;
};

template <typename C>
class const_slice_indexable : public slice {
public:
	typedef typename C::value_type value_type;

	explicit const_slice_indexable(const slice& s, const C& a): slice(s), _a(a) {}

	template <typename RHS>
	const_slice_indexable<C>& operator=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(unsigned int ii = 0; ii < size(); ++ii) {
			operator[](ii) = ExprT(rhs)[ii];
		}

		return *this;
	}

	value_type& operator[](const unsigned int& ii) {
		return _a[start()+ii*stride()];
	}

	const value_type& operator[](const unsigned int& ii) const {
		return _a[start()+ii*stride()];
	}

private:
	const C& _a;
};

template <typename C>
struct indexable_traits<slice_indexable<C>> {
	static const bool conforms = true;
};

template <typename C>
struct indexable_traits<const_slice_indexable<C>> {
	static const bool conforms = true;
};

} // smt

#endif // _SLICABLE_H
