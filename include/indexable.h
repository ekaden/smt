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

#ifndef _INDEXABLE_H
#define _INDEXABLE_H

#include <cmath>
#include <cstddef>
#include <type_traits>

#include "expression.h"
#include "operator.h"

namespace smt {

//
// Järvi J, Willcock J, and Lumsdaine A: Concept-Controlled Polymorphism. In:
// Generative Programming and Component Engineering, pp. 228–244, 2003.
//

// TODO: unsigned int => std::size_t

template <typename I, typename E = void>
struct indexable_traits {
	static const bool conforms = false;
};

template <typename I>
struct indexable {
	static_assert(indexable_traits<I>::conforms, "indexable_traits<I>::conforms");

	static typename I::value_type index(const I& idx, const unsigned int& ii) {
		return indexable_traits<I>::index(idx, ii);
	}

	static typename I::value_type& index(I& idx, const unsigned int& ii) {
		return indexable_traits<I>::index(idx, ii);
	}

	static unsigned int size(const I& idx) {
		return indexable_traits<I>::size(idx);
	}
};

#ifndef DEFINE_UNARY_FUNCTION
#define DEFINE_UNARY_FUNCTION(Func, FuncObj) \
	template <typename A> \
	typename std::enable_if< \
			smt::expr_traits<A>::conforms || indexable_traits<A>::conforms, \
			smt::expr<smt::expr_unary_op< \
				typename std::conditional<smt::expr_traits<A>::conforms, A, typename std::conditional<indexable_traits<A>::conforms, smt::constref<A>, smt::scalar<A>>::type>::type, \
				FuncObj>>>::type \
	Func(const A& a) { \
		typedef smt::expr_unary_op< \
				typename std::conditional<smt::expr_traits<A>::conforms, A, typename std::conditional<indexable_traits<A>::conforms, smt::constref<A>, smt::scalar<A>>::type>::type, \
				FuncObj> expr_type; \
		 \
		return smt::expr<expr_type>(expr_type( \
				typename std::conditional<smt::expr_traits<A>::conforms, A, typename std::conditional<indexable_traits<A>::conforms, smt::constref<A>, smt::scalar<A>>::type>::type(a))); \
	}

DEFINE_UNARY_FUNCTION(operator~, smt::bitwise_not)
DEFINE_UNARY_FUNCTION(operator!, smt::logical_not)
DEFINE_UNARY_FUNCTION(operator-, smt::negate)
DEFINE_UNARY_FUNCTION(operator+, smt::not_negate)

DEFINE_UNARY_FUNCTION(abs, smt::abs_function)
DEFINE_UNARY_FUNCTION(acos, smt::acos_function)
DEFINE_UNARY_FUNCTION(asin, smt::asin_function)
DEFINE_UNARY_FUNCTION(atan, smt::atan_function)
DEFINE_UNARY_FUNCTION(ceil, smt::ceil_function)
DEFINE_UNARY_FUNCTION(cos, smt::cos_function)
DEFINE_UNARY_FUNCTION(cosh, smt::cosh_function)
DEFINE_UNARY_FUNCTION(exp, smt::exp_function)
DEFINE_UNARY_FUNCTION(fabs, smt::fabs_function)
DEFINE_UNARY_FUNCTION(floor, smt::floor_function)
DEFINE_UNARY_FUNCTION(log, smt::log_function)
DEFINE_UNARY_FUNCTION(log10, smt::log10_function)
DEFINE_UNARY_FUNCTION(pow2, smt::pow2_function)
DEFINE_UNARY_FUNCTION(sin, smt::sin_function)
DEFINE_UNARY_FUNCTION(sinh, smt::sinh_function)
DEFINE_UNARY_FUNCTION(sqrt, smt::sqrt_function)
DEFINE_UNARY_FUNCTION(tan, smt::tan_function)
DEFINE_UNARY_FUNCTION(tanh, smt::tanh_function)

#undef DEFINE_UNARY_FUNCTION
#endif

#ifndef DEFINE_BINARY_FUNCTION
#define DEFINE_BINARY_FUNCTION(Func, FuncObj) \
	template <typename A, typename B> \
	typename std::enable_if< \
			smt::expr_traits<A>::conforms || smt::expr_traits<B>::conforms || indexable_traits<A>::conforms || indexable_traits<B>::conforms, \
			smt::expr<smt::expr_binary_op< \
				typename std::conditional<smt::expr_traits<A>::conforms, A, typename std::conditional<indexable_traits<A>::conforms, smt::constref<A>, smt::scalar<A>>::type >::type, \
				typename std::conditional<smt::expr_traits<B>::conforms, B, typename std::conditional<indexable_traits<B>::conforms, smt::constref<B>, smt::scalar<B>>::type >::type, \
				FuncObj>>>::type \
	Func(const A& a, const B& b) { \
		typedef smt::expr_binary_op< \
				typename std::conditional<smt::expr_traits<A>::conforms, A, typename std::conditional<indexable_traits<A>::conforms, smt::constref<A>, smt::scalar<A>>::type >::type, \
				typename std::conditional<smt::expr_traits<B>::conforms, B, typename std::conditional<indexable_traits<B>::conforms, smt::constref<B>, smt::scalar<B>>::type >::type, \
				FuncObj> expr_type; \
		 \
		return smt::expr<expr_type>(expr_type( \
				typename std::conditional<smt::expr_traits<A>::conforms, A, typename std::conditional<indexable_traits<A>::conforms, smt::constref<A>, smt::scalar<A>>::type >::type(a), \
				typename std::conditional<smt::expr_traits<B>::conforms, B, typename std::conditional<indexable_traits<B>::conforms, smt::constref<B>, smt::scalar<B>>::type >::type(b))); \
	}

DEFINE_BINARY_FUNCTION(operator&, smt::bitwise_and)
DEFINE_BINARY_FUNCTION(operator|, smt::bitwise_or)
DEFINE_BINARY_FUNCTION(operator^, smt::bitwise_xor)
DEFINE_BINARY_FUNCTION(operator/, smt::divides)
DEFINE_BINARY_FUNCTION(operator==, smt::equal_to)
DEFINE_BINARY_FUNCTION(operator>, smt::greater)
DEFINE_BINARY_FUNCTION(operator>=, smt::greater_equal)
DEFINE_BINARY_FUNCTION(operator<, smt::less)
DEFINE_BINARY_FUNCTION(operator<=, smt::less_equal)
DEFINE_BINARY_FUNCTION(operator&&, smt::logical_and)
DEFINE_BINARY_FUNCTION(operator||, smt::logical_or)
DEFINE_BINARY_FUNCTION(operator-, smt::minus)
DEFINE_BINARY_FUNCTION(operator%, smt::modulus)
DEFINE_BINARY_FUNCTION(operator*, smt::multiplies)
DEFINE_BINARY_FUNCTION(operator!=, smt::not_equal_to)
DEFINE_BINARY_FUNCTION(operator+, smt::plus)
DEFINE_BINARY_FUNCTION(operator<<, smt::shift_left)
DEFINE_BINARY_FUNCTION(operator>>, smt::shift_right)

DEFINE_BINARY_FUNCTION(atan2, smt::atan2_function)
DEFINE_BINARY_FUNCTION(fmod, smt::fmod_function)
DEFINE_BINARY_FUNCTION(frexp, smt::frexp_function)
DEFINE_BINARY_FUNCTION(ldexp, smt::ldexp_function)
DEFINE_BINARY_FUNCTION(modf, smt::modf_function)
DEFINE_BINARY_FUNCTION(pow, smt::pow_function)
DEFINE_BINARY_FUNCTION(min, smt::min_function)
DEFINE_BINARY_FUNCTION(max, smt::max_function)

#undef DEFINE_BINARY_FUNCTION
#endif

#ifndef DEFINE_TRINARY_FUNCTION
#define DEFINE_TRINARY_FUNCTION(Func, FuncObj) \
	template <typename A, typename B, typename C> \
	typename std::enable_if< \
			smt::expr_traits<A>::conforms || smt::expr_traits<B>::conforms || smt::expr_traits<C>::conforms || indexable_traits<A>::conforms || indexable_traits<B>::conforms || indexable_traits<C>::conforms, \
			smt::expr<smt::expr_trinary_op< \
				typename std::conditional<smt::expr_traits<A>::conforms, A, typename std::conditional<indexable_traits<A>::conforms, smt::constref<A>, smt::scalar<A>>::type >::type, \
				typename std::conditional<smt::expr_traits<B>::conforms, B, typename std::conditional<indexable_traits<B>::conforms, smt::constref<B>, smt::scalar<B>>::type >::type, \
				typename std::conditional<smt::expr_traits<C>::conforms, C, typename std::conditional<indexable_traits<C>::conforms, smt::constref<C>, smt::scalar<C>>::type >::type, \
				FuncObj>>>::type \
	Func(const A& a, const B& b, const C& c) { \
		typedef smt::expr_trinary_op< \
				typename std::conditional<smt::expr_traits<A>::conforms, A, typename std::conditional<indexable_traits<A>::conforms, smt::constref<A>, smt::scalar<A>>::type >::type, \
				typename std::conditional<smt::expr_traits<B>::conforms, B, typename std::conditional<indexable_traits<B>::conforms, smt::constref<B>, smt::scalar<B>>::type >::type, \
				typename std::conditional<smt::expr_traits<C>::conforms, C, typename std::conditional<indexable_traits<C>::conforms, smt::constref<C>, smt::scalar<C>>::type >::type, \
				FuncObj> expr_type; \
		 \
		return smt::expr<expr_type>(expr_type( \
				typename std::conditional<smt::expr_traits<A>::conforms, A, typename std::conditional<indexable_traits<A>::conforms, smt::constref<A>, smt::scalar<A>>::type >::type(a), \
				typename std::conditional<smt::expr_traits<B>::conforms, B, typename std::conditional<indexable_traits<B>::conforms, smt::constref<B>, smt::scalar<B>>::type >::type(b), \
				typename std::conditional<smt::expr_traits<C>::conforms, C, typename std::conditional<indexable_traits<C>::conforms, smt::constref<C>, smt::scalar<C>>::type >::type(c))); \
	}

DEFINE_TRINARY_FUNCTION(where, smt::where_function)

#undef DEFINE_TRINARY_FUNCTION
#endif

} // smt

#endif // _INDEXABLE_H
