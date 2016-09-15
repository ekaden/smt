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

#ifndef _DARRAY_H
#define _DARRAY_H

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

template <typename T, unsigned int D>
class darray {
public:
	typedef T value_type;
	typedef unsigned int size_type;

	typedef T* iterator;
	typedef const T* const_iterator;

	typedef T& reference;
	typedef const T& const_reference;

	darray(): _rc(false), _data(0) {
		for(unsigned int ii = 0; ii < D; ++ii) {
			_size[ii] = 0;
		}
	}

	darray(const darray<T, 1>& rhs):
			_rc(true), _data(new T[rhs.size(0)]) {
		static_assert(D == 1, "D == 1");
		_size[0] = rhs.size(0);
		std::copy(rhs.begin(), rhs.end(), begin());
	}

	darray(const darray<T, 2>& rhs):
			_rc(true), _data(new T[rhs.size(0)*rhs.size(1)]) {
		static_assert(D == 2, "D == 2");
		_size[0] = rhs.size(0);
		_size[1] = rhs.size(1);
		std::copy(rhs.begin(), rhs.end(), begin());
	}

	darray(const darray<T, 3>& rhs):
			_rc(true), _data(new T[rhs.size(0)*rhs.size(1)*rhs.size(2)]) {
		static_assert(D == 3, "D == 3");
		_size[0] = rhs.size(0);
		_size[1] = rhs.size(1);
		_size[2] = rhs.size(2);
		std::copy(rhs.begin(), rhs.end(), begin());
	}

	darray(const darray<T, 4>& rhs):
			_rc(true), _data(new T[rhs.size(0)*rhs.size(1)*rhs.size(2)*rhs.size(3)]) {
		static_assert(D == 4, "D == 4");
		_size[0] = rhs.size(0);
		_size[1] = rhs.size(1);
		_size[2] = rhs.size(2);
		_size[3] = rhs.size(3);
		std::copy(rhs.begin(), rhs.end(), begin());
	}

	explicit darray(const size_type& s0):
			_rc(true), _data(new T[s0]) {
		static_assert(D == 1, "D == 1");
		_size[0] = s0;
	}

	explicit darray(const size_type& s0, T *const data):
			_rc(false), _data(data) {
		static_assert(D == 1, "D == 1");
		_size[0] = s0;
	}

	explicit darray(const size_type& s0, const size_type& s1):
			_rc(true), _data(new T[s0*s1]) {
		static_assert(D == 2, "D == 2");
		_size[0] = s0;
		_size[1] = s1;
	}

	explicit darray(const size_type& s0, const size_type& s1, T *const data):
			_rc(false), _data(data) {
		static_assert(D == 2, "D == 2");
		_size[0] = s0;
		_size[1] = s1;
	}

	explicit darray(const size_type& s0, const size_type& s1,
			const size_type& s2):
			_rc(true), _data(new T[s0*s1*s2]) {
		static_assert(D == 3, "D == 3");
		_size[0] = s0;
		_size[1] = s1;
		_size[2] = s2;
	}

	explicit darray(const size_type& s0, const size_type& s1,
			const size_type& s2, T *const data):
			_rc(false), _data(data) {
		static_assert(D == 3, "D == 3");
		_size[0] = s0;
		_size[1] = s1;
		_size[2] = s2;
	}

	explicit darray(const size_type& s0, const size_type& s1,
			const size_type& s2, const size_type& s3):
			_rc(true), _data(new T[s0*s1*s2*s3]) {
		static_assert(D == 4, "D == 4");
		_size[0] = s0;
		_size[1] = s1;
		_size[2] = s2;
		_size[3] = s3;
	}

	explicit darray(const size_type& s0, const size_type& s1,
			const size_type& s2, const size_type& s3, T *const data):
			_rc(false), _data(data) {
		static_assert(D == 4, "D == 4");
		_size[0] = s0;
		_size[1] = s1;
		_size[2] = s2;
		_size[3] = s3;
	}

	~darray() {
		if(_rc) {
			delete [] _data;
			_data = nullptr;
		}
		_rc = false;
		for(unsigned int ii = 0; ii < D; ++ii) {
			_size[ii] = 0;
		}
	}

	explicit operator bool() const {
		return size() != 0;
	}

	darray<T, D>& operator=(const darray<T, D>& rhs) {
		for(size_type ii = 0; ii < size(); ++ii) {
			operator[](ii) = rhs[ii];
		}

		return *this;
	}

	template <typename RHS>
	darray<T, D>& operator=(const RHS& rhs) {
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
	darray<T, D>& operator+=(const RHS& rhs) {
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
	darray<T, D>& operator-=(const RHS& rhs) {
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
			operator[](ii) -= ExprT(rhs)[ii];
		}

		return *this;
	}

	template <typename RHS>
	darray<T, D>& operator*=(const RHS& rhs) {
		typedef typename std::conditional<
				smt::expr_traits<RHS>::conforms,
				RHS,
				typename std::conditional<
						smt::indexable_traits<RHS>::conforms,
						smt::constref<RHS>,
						smt::scalar<RHS>
				>::type
		>::type ExprT;

		for(size_type i = 0; i < size(); ++i) operator[](i) *= ExprT(rhs)[i];
		return *this;
	}

	template <typename RHS>
	darray<T, D>& operator/=(const RHS& rhs) {
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
		return _data+size();
	}

	const_iterator end() const {
		return _data+size();
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

	slice_indexable<darray<T, D>> operator[](const slice& s) {
		return slice_indexable<darray<T, D>>(s, *this);
	}

	const_slice_indexable<darray<T, D>> operator[](const slice& s) const {
		return const_slice_indexable<darray<T, D>>(s, *this);
	}

	reference operator()(const size_type& i0) {
		static_assert(D == 1, "D == 1");
		insist(0 <= i0 && i0 < size(0));
		return _data[i0];
	}

	reference colmaj(const size_type& i0) {
		static_assert(D == 1, "D == 1");
		insist(0 <= i0 && i0 < size(0));
		return _data[i0];
	}

	const_reference operator()(const size_type& i0) const {
		static_assert(D == 1, "D == 1");
		insist(0 <= i0 && i0 < size(0));
		return _data[i0];
	}

	const_reference colmaj(const size_type& i0) const {
		static_assert(D == 1, "D == 1");
		insist(0 <= i0 && i0 < size(0));
		return _data[i0];
	}

	reference operator()(const size_type& i0, const size_type& i1) {
		static_assert(D == 2, "D == 2");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1));
		return _data[i0*size(1)+i1];
	}

	reference colmaj(const size_type& i0, const size_type& i1) {
		static_assert(D == 2, "D == 2");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1));
		return _data[i0+size(0)*i1];
	}

	const_reference operator()(const size_type& i0, const size_type& i1) const {
		static_assert(D == 2, "D == 2");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1));
		return _data[i0*size(1)+i1];
	}

	const_reference colmaj(const size_type& i0, const size_type& i1) const {
		static_assert(D == 2, "D == 2");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1));
		return _data[i0+size(0)*i1];
	}

	reference operator()(const size_type& i0, const size_type& i1,
			const size_type& i2) {
		static_assert(D == 3, "D == 3");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1)
				&& 0 <= i2 && i2 < size(2));
		return _data[(i0*size(1)+i1)*size(2)+i2];
	}

	reference colmaj(const size_type& i0, const size_type& i1,
			const size_type& i2) {
		static_assert(D == 3, "D == 3");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1)
				&& 0 <= i2 && i2 < size(2));
		return _data[i0+size(0)*(i1+size(1)*i2)];
	}

	const_reference operator()(const size_type& i0, const size_type& i1,
			const size_type& i2) const {
		static_assert(D == 3, "D == 3");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1)
				&& 0 <= i2 && i2 < size(2));
		return _data[(i0*size(1)+i1)*size(2)+i2];
	}

	const_reference colmaj(const size_type& i0, const size_type& i1,
			const size_type& i2) const {
		static_assert(D == 3, "D == 3");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1)
				&& 0 <= i2 && i2 < size(2));
		return _data[i0+size(0)*(i1+size(1)*i2)];
	}

	reference operator()(const size_type& i0, const size_type& i1,
			const size_type& i2, const size_type& i3) {
		static_assert(D == 4, "D == 4");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1)
				&& 0 <= i2 && i2 < size(2) && 0 <= i3 && i3 < size(3));
		return _data[((i0*size(1)+i1)*size(2)+i2)*size(3)+i3];
	}

	reference colmaj(const size_type& i0, const size_type& i1,
			const size_type& i2, const size_type& i3) {
		static_assert(D == 4, "D == 4");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1)
				&& 0 <= i2 && i2 < size(2) && 0 <= i3 && i3 < size(3));
		return _data[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
	}

	const_reference operator()(const size_type& i0, const size_type& i1,
			const size_type& i2, const size_type& i3) const {
		static_assert(D == 4, "D == 4");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1)
				&& 0 <= i2 && i2 < size(2) && 0 <= i3 && i3 < size(3));
		return _data[((i0*size(1)+i1)*size(2)+i2)*size(3)+i3];
	}

	const_reference colmaj(const size_type& i0, const size_type& i1,
			const size_type& i2, const size_type& i3) const {
		static_assert(D == 4, "D == 4");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1)
				&& 0 <= i2 && i2 < size(2) && 0 <= i3 && i3 < size(3));
		return _data[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
	}

	void resize(const size_type& s0) {
		static_assert(D == 1, "D == 1");
		if(_rc) {
			delete [] _data;
		}
		_rc = true;
		_size[0] = s0;
		_data = new T[s0];
	}

	void resize(const size_type& s0, T *const data) {
		static_assert(D == 1, "D == 1");
		if(_rc) {
			delete [] _data;
		}
		_rc = false;
		_size[0] = s0;
		_data = data;
	}

	void resize(const size_type& s0, const size_type& s1) {
		static_assert(D == 2, "D == 2");
		if(_rc) {
			delete [] _data;
		}
		_rc = true;
		_size[0] = s0;
		_size[1] = s1;
		_data = new T[s0*s1];
	}

	void resize(const size_type& s0, const size_type& s1, T *const data) {
		static_assert(D == 2, "D == 2");
		if(_rc) {
			delete [] _data;
		}
		_rc = false;
		_size[0] = s0;
		_size[1] = s1;
		_data = data;
	}

	void resize(const size_type& s0, const size_type& s1,
			const size_type& s2) {
		static_assert(D == 3, "D == 3");
		if(_rc) {
			delete [] _data;
		}
		_rc = true;
		_size[0] = s0;
		_size[1] = s1;
		_size[2] = s2;
		_data = new T[s0*s1*s2];
	}

	void resize(const size_type& s0, const size_type& s1,
			const size_type& s2, T *const data) {
		static_assert(D == 3, "D == 3");
		if(_rc) {
			delete [] _data;
		}
		_rc = false;
		_size[0] = s0;
		_size[1] = s1;
		_size[2] = s2;
		_data = data;
	}

	void resize(const size_type& s0, const size_type& s1,
			const size_type& s2, const size_type& s3) {
		static_assert(D == 4, "D == 4");
		if(_rc) {
			delete [] _data;
		}
		_rc = true;
		_size[0] = s0;
		_size[1] = s1;
		_size[2] = s2;
		_size[3] = s3;
		_data = new T[s0*s1*s2*s3];
	}

	void resize(const size_type& s0, const size_type& s1,
			const size_type& s2, const size_type& s3, T *const data) {
		static_assert(D == 4, "D == 4");
		if(_rc) {
			delete [] _data;
		}
		_rc = false;
		_size[0] = s0;
		_size[1] = s1;
		_size[2] = s2;
		_size[3] = s3;
		_data = data;
	}

	size_type size() const {
		size_type total_size = 1;
		for(unsigned int ii = 0; ii < D; ++ii) {
			total_size *= _size[ii];
		}
		return total_size;
	}

	size_type size(const size_type& ii) const {
		insist(0 <= ii && ii < D);
		return _size[ii];
	}

private:
	bool _rc;
	size_type _size[D];
	T* _data;
};

template <typename T, unsigned int D>
struct indexable_traits<darray<T, D>> {
	static const bool conforms = true;

	static typename darray<T, D>::value_type index(
			const darray<T, D>& a, const unsigned int& ii) {
		return a[ii];
	}

	static typename darray<T, D>::value_type& index(
			darray<T, D>& a, const unsigned int& ii) {
		return a[ii];
	}

	static unsigned int size(const darray<T, D>& a) {
		return a.size();
	}
};

template <typename T>
darray<T, 1> gemv(const darray<T, 2>& A, const darray<T, 1>& x) {
	darray<T, 1> y(A.size(0));
	y = 0;
	for(unsigned int ii = 0; ii < A.size(0); ++ii) {
		for(unsigned int jj = 0; jj < A.size(1); ++jj) {
			y(ii) += A(ii, jj)*x(jj);
		}
	}

	return y;
}

template <typename T>
T dot(const darray<T, 1>& x, const darray<T, 1>& y) {
	T ret = 0;
	for(unsigned int ii = 0; ii < x.size(0); ++ii) {
		ret += x(ii)*y(ii);
	}

	return ret;
}

template <typename T>
T norm1(const darray<T, 1>& x) {
	T ret = 0;
	for(std::size_t ii = 0; ii < x.size(0); ++ii) {
		ret += std::abs(x(ii));
	}

	return ret;
}

template <typename T>
T norm2(const darray<T, 1>& x) {
	T ret = 0;
	for(std::size_t ii = 0; ii < x.size(0); ++ii) {
		ret += smt::pow2(x(ii));
	}

	return std::sqrt(ret);
}

template <typename T>
T normInf(const darray<T, 1>& x) {
	T ret = 0;
	for(std::size_t ii = 0; ii < x.size(0); ++ii) {
		ret = std::max(ret, std::abs(x(ii)));
	}

	return ret;
}

template <typename T>
darray<T, 2> eye(const std::size_t& s0, const std::size_t& s1) {
	darray<T, 2> ret(s0, s1);
	ret = 0;
	for(std::size_t ii = 0; ii < std::min(s0, s1); ++ii) {
		ret(ii, ii) = T(1);
	}

	return ret;
}

} // smt

template <typename T>
std::ostream& operator<<(std::ostream& s, const smt::darray<T, 1>& rhs) {
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

template <typename T>
std::ostream& operator<<(std::ostream& s, const smt::darray<T, 2>& rhs) {
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

#endif // _DARRAY_H
