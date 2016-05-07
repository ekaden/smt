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

#ifndef _SARRAY_H
#define _SARRAY_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ostream>
#include <type_traits>

#include "debug.h"
#include "indexable.h"
#include "slicable.h"

namespace smt {

// TODO: unsigned int => std::size_t, signed int => std::ptrdiff_t
// TODO: Switch from row-major to column-major order.

template <typename T, unsigned int S0, unsigned int S1 = 0>
class sarray;

template <typename T, unsigned int S0>
class sarray<T, S0> {
public:
	typedef T value_type;
	typedef unsigned int size_type;

	typedef T* iterator;
	typedef const T* const_iterator;

	typedef T& reference;
	typedef const T& const_reference;

	sarray() {}

	template <typename RHS>
	sarray(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type ii = 0; ii < size(0); ++ii) {
			operator[](ii) = ExprT(rhs)[ii];
		}
	}

	explicit operator bool() const {
		return size() != 0;
	}

	template <typename RHS>
	sarray<T, S0>& operator=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type ii = 0; ii < size(0); ++ii) {
			operator[](ii) = ExprT(rhs)[ii];
		}

		return *this;
	}
	
	template <typename RHS>
	sarray<T, S0>& operator+=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type ii = 0; ii < size(0); ++ii) {
			operator[](ii) += ExprT(rhs)[ii];
		}

		return *this;
	}
	
	template <typename RHS>
	sarray<T, S0>& operator-=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type ii = 0; ii < size(0); ++ii) {
			operator[](ii) -= ExprT(rhs)[ii];
		}

		return *this;
	}
	
	template <typename RHS>
	sarray<T, S0>& operator*=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type ii = 0; ii < size(0); ++ii) {
			operator[](ii) *= ExprT(rhs)[ii];
		}

		return *this;
	}
	
	template <typename RHS>
	sarray<T, S0>& operator/=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type ii = 0; ii < size(0); ++ii) {
			operator[](ii) /= ExprT(rhs)[ii];
		}

		return *this;
	}

	iterator begin() {
		return _data;
	}

	const_iterator begin() const {
		return _data;
	}

	iterator end() {
		return _data+S0;
	}

	const_iterator end() const {
		return _data+S0;
	}

	reference front() {
		return _data[0];
	}

	const_reference front() const {
		return _data[0];
	}

	reference back() {
		insist(0 < size());
		return _data[size()-1];
	}

	const_reference back() const {
		insist(0 < size());
		return _data[size()-1];
	}

	reference operator[](const size_type& ii) {
		insist(0 <= ii && ii < size());
		return _data[ii];
	}

	const_reference operator[](const size_type& ii) const {
		insist(0 <= ii && ii < size());
		return _data[ii];
	}

	slice_indexable<sarray<T, S0>> operator[](const slice& s) {
		return slice_indexable<sarray<T, S0>>(s, *this);
	}

	const_slice_indexable<sarray<T, S0>> operator[](const slice& s) const {
		return const_slice_indexable<sarray<T, S0>>(s, *this);
	}

	reference operator()(const size_type& i0) {
		insist(0 <= i0 && i0 < size(0));
		return _data[i0];
	}

	reference colmaj(const size_type& i0) {
		insist(0 <= i0 && i0 < size(0));
		return _data[i0];
	}

	const_reference operator()(const size_type& i0) const {
		insist(0 <= i0 && i0 < size(0));
		return _data[i0];
	}

	const_reference colmaj(const size_type& i0) const {
		insist(0 <= i0 && i0 < size(0));
		return _data[i0];
	}

	size_type size() const {
		return S0;
	}

	size_type size(const size_type& ii) const {
		insist(0 <= ii && ii < 1);
		return S0;
	}

private:
	T _data[S0];
};

template <typename T, unsigned int S0, unsigned int S1>
class sarray {
public:
	typedef T value_type;
	typedef unsigned int size_type;

	typedef T* iterator;
	typedef const T* const_iterator;

	typedef T& reference;
	typedef const T& const_reference;

	sarray() {}

	template <typename RHS>
	sarray(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type ii = 0; ii < size(); ++ii) {
			operator[](ii) = ExprT(rhs)[ii];
		}
	}

	explicit operator bool() const {
		return size() != 0;
	}

	template <typename RHS>
	sarray<T, S0, S1>& operator=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type ii = 0; ii < size(); ++ii) {
			operator[](ii) = ExprT(rhs)[ii];
		}

		return *this;
	}

	template <typename RHS>
	sarray<T, S0, S1>& operator+=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type ii = 0; ii < size(); ++ii) {
			operator[](ii) += ExprT(rhs)[ii];
		}

		return *this;
	}

	template <typename RHS>
	sarray<T, S0, S1>& operator*=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type ii = 0; ii < size(); ++ii) {
			operator[](ii) *= ExprT(rhs)[ii];
		}

		return *this;
	}

	template <typename RHS>
	sarray<T, S0, S1>& operator/=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type ii = 0; ii < size(); ++ii) {
			operator[](ii) /= ExprT(rhs)[ii];
		}

		return *this;
	}

	iterator begin() {
		return _data;
	}

	const_iterator begin() const {
		return _data;
	}

	iterator end() {
		return _data+S0*S1;
	}

	const_iterator end() const {
		return _data+S0*S1;
	}

	reference front() {
		return _data[0];
	}

	const_reference front() const {
		return _data[0];
	}

	reference back() {
		insist(0 < size());
		return _data[size()-1];
	}

	const_reference back() const {
		insist(0 < size());
		return _data[size()-1];
	}

	reference operator[](const size_type& ii) {
		insist(0 <= ii && ii < size());
		return _data[ii];
	}

	const_reference operator[](const size_type& ii) const {
		insist(0 <= ii && ii < size());
		return _data[ii];
	}

	slice_indexable<sarray<T, S0, S1>> operator[](const slice& s) {
		return slice_indexable<sarray<T, S0, S1>>(s, *this);
	}

	const_slice_indexable<sarray<T, S0, S1>> operator[](const slice& s) const {
		return const_slice_indexable<sarray<T, S0, S1>>(s, *this);
	}

	reference operator()(const size_type& i0, const size_type& i1) {
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1));
		return _data[i0*size(1)+i1];
	}

	reference colmaj(const size_type& i0, const size_type& i1) {
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1));
		return _data[i0+size(0)*i1];
	}

	const_reference operator()(const size_type& i0, const size_type& i1) const {
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1));
		return _data[i0*size(1)+i1];
	}

	const_reference colmaj(const size_type& i0, const size_type& i1) const {
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1));
		return _data[i0+size(0)*i1];
	}

	size_type size() const {
		return S0*S1;
	}

	size_type size(const size_type& ii) const {
		insist(0 <= ii && ii < 2);
		return (ii == 0)? S0 : S1;
	}

private:
	T _data[S0*S1];
};

template <typename T, unsigned int S0, unsigned int S1>
struct indexable_traits<sarray<T, S0, S1>> {
	static const bool conforms = true;

	static typename sarray<T, S0, S1>::value_type index(
			const sarray<T, S0, S1>& a, const unsigned int& ii) {
		return a[ii];
	}

	static typename sarray<T, S0, S1>::value_type& index(
			sarray<T, S0, S1>& a, const unsigned int& ii) {
		return a[ii];
	}

	static unsigned int size(const sarray<T, S0, S1>& a) {
		return a.size();
	}
};

template <typename T, unsigned int S0, unsigned int S1>
sarray<T, S0> mvProduct(const sarray<T, S0, S1>& A, const sarray<T, S1>& x) {
	sarray<T, S0> y = 0;
	for(unsigned int ii = 0; ii < A.size(0); ++ii) {
		for(unsigned int jj = 0; jj < A.size(1); ++jj) {
			y(ii) += A(ii, jj)*x(jj);
		}
	}

	return y;
}

template <typename T, unsigned int S0>
T scalarProduct(const sarray<T, S0>& x, const sarray<T, S0>& y) {
	T ret = 0;
	for(unsigned int ii = 0; ii < x.size(0); ++ii) {
		ret += x(ii)*y(ii);
	}

	return ret;
}

template <typename T, unsigned int S0>
T norm1(const sarray<T, S0>& x) {
	T ret = 0;
	for(std::size_t ii = 0; ii < S0; ++ii) {
		ret += std::abs(x(ii));
	}

	return ret;
}

template <typename T, unsigned int S0>
T norm2(const sarray<T, S0>& x) {
	T ret = 0;
	for(std::size_t ii = 0; ii < S0; ++ii) {
		ret += smt::pow2(x(ii));
	}

	return std::sqrt(ret);
}

template <typename T, unsigned int S0>
T normInf(const sarray<T, S0>& x) {
	T ret = 0;
	for(std::size_t ii = 0; ii < S0; ++ii) {
		ret = std::max(ret, std::abs(x(ii)));
	}

	return ret;
}

} // smt

template <typename T, unsigned int S0>
std::ostream& operator<<(std::ostream& s, const smt::sarray<T, S0>& rhs) {
	s << "[";
	if(rhs.size() > 0) {
		for(std::size_t ii = 0; ii < rhs.size()-1; ++ii) {
			s << rhs(ii) << ", ";
		}
		s << rhs[rhs.size()-1];
	}
	s << "]";

	return s;
}

template <typename T, unsigned int S0, unsigned int S1>
std::ostream& operator<<(std::ostream& s, const smt::sarray<T, S0, S1>& rhs) {
	s << "[";
	if(rhs.size(0) > 0) {
		for(std::size_t ii = 0; ii < rhs.size(0)-1; ++ii) {
			s << "[";
			if(rhs.size(1) > 0) {
				for(std::size_t jj = 0; jj < rhs.size(1)-1; ++jj) {
					s << rhs(ii, jj) << ", ";
				}
				s << rhs(ii, rhs.size(1)-1);
			}
			s << "], ";
		}
		s << "[";
		if(rhs.size(1) > 0) {
			for(std::size_t jj = 0; jj < rhs.size(1)-1; ++jj) {
				s << rhs(rhs.size(0)-1, jj) << ", ";
			}
			s << rhs(rhs.size(0)-1, rhs.size(1)-1);
		}
		s << "]";
	}
	s << "]";

	return s;
}

#endif // _SARRAY_H
