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

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <cstddef>

#include "operator.h"

namespace smt {

//
// Järvi J, Willcock J, and Lumsdaine A: Concept-Controlled Polymorphism. In:
// Generative Programming and Component Engineering, pp. 228–244, 2003.
//

// TODO: unsigned int => std::size_t

template <typename I, typename E = void>
struct expr_traits {
	static const bool conforms = false;
};

template <typename ExprT>
class expr {
public:
	typedef typename ExprT::value_type value_type;

	explicit expr(const ExprT& e): _e(e) {}

	const value_type operator[](const unsigned int& ii) const {
		return _e[ii];
	}

private:
	ExprT _e;
};

template <typename ExprT>
struct expr_traits<expr<ExprT>> {
	static const bool conforms = true;
};

template <typename T>
class scalar {
public:
	typedef T value_type;

	explicit scalar(const T& s): _s(s) {}

	const value_type operator[](const unsigned int& ii) const {
		return _s;
	}

private:
	T _s;
};

template <typename ContainerT>
class constref {
public:
	typedef typename ContainerT::value_type value_type;

	explicit constref(const ContainerT& c): _c(c) {}

	const value_type operator[](const unsigned int& ii) const {
		return _c[ii];
	}

private:
	const ContainerT& _c;
};

template <typename A, typename Op>
class expr_unary_op {
public:
	typedef typename smt::UnaryReturn<typename A::value_type, Op>::type value_type;

	explicit expr_unary_op(const A& a): _a(a) {}

	const value_type operator[](const unsigned int& ii) const {
		return Op::apply(_a[ii]);
	}

private:
	A _a;
};

template <typename A, typename B, typename Op>
class expr_binary_op {
public:
	typedef typename smt::BinaryReturn<
			typename A::value_type,
			typename B::value_type,
			Op
		>::type value_type;

	explicit expr_binary_op(const A& a, const B& b): _a(a), _b(b) {}

	const value_type operator[](const unsigned int& ii) const {
		return Op::apply(_a[ii], _b[ii]);
	}

private:
	A _a;
	B _b;
};

template <typename A, typename B, typename C, typename Op>
class expr_trinary_op {
public:
	typedef typename smt::TrinaryReturn<
			typename A::value_type,
			typename B::value_type,
			typename C::value_type,
			Op
		>::type value_type;

	explicit expr_trinary_op(const A& a, const B& b, const C& c):
			_a(a), _b(b), _c(c) {}

	const value_type operator[](const unsigned int& ii) const {
		return Op::apply(_a[ii], _b[ii], _c[ii]);
	}

private:
	A _a;
	B _b;
	C _c;
};

} // smt

#endif // _EXPRESSION_H
