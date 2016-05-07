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

#ifndef _OPERATOR_H
#define _OPERATOR_H

#include <algorithm>
#include <cmath>
#include <type_traits>

#include "pow.h"

namespace smt {

//
// Järvi J, Willcock J, and Lumsdaine A: Concept-Controlled Polymorphism. In:
// Generative Programming and Component Engineering, pp. 228–244, 2003.
//

template<typename T, typename Op>
struct UnaryReturn {
	typedef T type;
};

#ifndef DEFINE_UNARY_OPERATOR
#define DEFINE_UNARY_OPERATOR(OpObj, Op) \
	struct OpObj { \
		template <typename T> \
		static typename UnaryReturn<T, OpObj>::type apply(const T& a) { \
			return(Op a); \
		} \
	};

DEFINE_UNARY_OPERATOR(bitwise_not, ~)
DEFINE_UNARY_OPERATOR(logical_not, !)
DEFINE_UNARY_OPERATOR(negate, -)
DEFINE_UNARY_OPERATOR(not_negate, +)

#undef DEFINE_UNARY_OPERATOR
#endif

template <typename T>
struct UnaryReturn<T, logical_not> {
	typedef bool type;
};

#ifndef DEFINE_UNARY_FUNCTOR
#define DEFINE_UNARY_FUNCTOR(FuncObj, Func) \
	struct FuncObj { \
		template <typename T> \
		static typename UnaryReturn<T, FuncObj>::type apply(const T& a) { \
			return(Func(a)); \
		} \
	};

DEFINE_UNARY_FUNCTOR(abs_function, std::abs)
DEFINE_UNARY_FUNCTOR(acos_function, std::acos)
DEFINE_UNARY_FUNCTOR(asin_function, std::asin)
DEFINE_UNARY_FUNCTOR(atan_function, std::atan)
DEFINE_UNARY_FUNCTOR(ceil_function, std::ceil)
DEFINE_UNARY_FUNCTOR(cos_function, std::cos)
DEFINE_UNARY_FUNCTOR(cosh_function, std::cosh)
DEFINE_UNARY_FUNCTOR(exp_function, std::exp)
DEFINE_UNARY_FUNCTOR(fabs_function, std::fabs)
DEFINE_UNARY_FUNCTOR(floor_function, std::floor)
DEFINE_UNARY_FUNCTOR(log_function, std::log)
DEFINE_UNARY_FUNCTOR(log10_function, std::log10)
DEFINE_UNARY_FUNCTOR(pow2_function, smt::pow2)
DEFINE_UNARY_FUNCTOR(sin_function, std::sin)
DEFINE_UNARY_FUNCTOR(sinh_function, std::sinh)
DEFINE_UNARY_FUNCTOR(sqrt_function, std::sqrt)
DEFINE_UNARY_FUNCTOR(tan_function, std::tan)
DEFINE_UNARY_FUNCTOR(tanh_function, std::tanh)

#undef DEFINE_UNARY_FUNCTOR
#endif

template<typename T1, typename T2, typename Op>
struct BinaryReturn {
	typedef typename std::common_type<T1, T2>::type type;
};

#ifndef DEFINE_BINARY_OPERATOR
#define DEFINE_BINARY_OPERATOR(OpObj, Op) \
	struct OpObj { \
		template <typename T1, typename T2> \
		static typename BinaryReturn<T1, T2, OpObj>::type \
		apply(const T1& a, const T2& b) { \
			return(a Op b); \
		} \
	};

DEFINE_BINARY_OPERATOR(bitwise_and, &)
DEFINE_BINARY_OPERATOR(bitwise_or, |)
DEFINE_BINARY_OPERATOR(bitwise_xor, ^)
DEFINE_BINARY_OPERATOR(divides, /)
DEFINE_BINARY_OPERATOR(equal_to, ==)
DEFINE_BINARY_OPERATOR(greater, >)
DEFINE_BINARY_OPERATOR(greater_equal, >=)
DEFINE_BINARY_OPERATOR(less, <)
DEFINE_BINARY_OPERATOR(less_equal, <=)
DEFINE_BINARY_OPERATOR(logical_and, &&)
DEFINE_BINARY_OPERATOR(logical_or, ||)
DEFINE_BINARY_OPERATOR(minus, -)
DEFINE_BINARY_OPERATOR(modulus, %)
DEFINE_BINARY_OPERATOR(multiplies, *)
DEFINE_BINARY_OPERATOR(not_equal_to, !=)
DEFINE_BINARY_OPERATOR(plus, +)
DEFINE_BINARY_OPERATOR(shift_left, <<)
DEFINE_BINARY_OPERATOR(shift_right, >>)

#undef DEFINE_BINARY_OPERATOR
#endif

#ifndef DEFINE_BINARY_FUNCTOR
#define DEFINE_BINARY_FUNCTOR(FuncObj, Func) \
	struct FuncObj { \
		template <typename T1, typename T2> \
		static typename BinaryReturn<T1, T2, FuncObj>::type \
		apply(const T1& a, const T2& b) { \
			return(Func(a, b)); \
		} \
	};

DEFINE_BINARY_FUNCTOR(atan2_function, std::atan2)
DEFINE_BINARY_FUNCTOR(fmod_function, std::fmod)
DEFINE_BINARY_FUNCTOR(frexp_function, std::frexp)
DEFINE_BINARY_FUNCTOR(ldexp_function, std::ldexp)
DEFINE_BINARY_FUNCTOR(modf_function, std::modf)
DEFINE_BINARY_FUNCTOR(pow_function, std::pow)
DEFINE_BINARY_FUNCTOR(min_function, std::min)
DEFINE_BINARY_FUNCTOR(max_function, std::max)

#undef DEFINE_BINARY_FUNCTOR
#endif

#ifndef DEFINE_BINARY_ASSIGNMENT
#define DEFINE_BINARY_ASSIGNMENT(OpObj, Op) \
	struct OpObj { \
		template <typename T1, typename T2> \
		static typename BinaryReturn<T1, T2, OpObj>::type \
		apply(const T1& a, const T2& b) { \
			return(const_cast<T1&>(a) Op b); \
		} \
	};

DEFINE_BINARY_ASSIGNMENT(assign, =)
DEFINE_BINARY_ASSIGNMENT(bitwise_and_assign, &=)
DEFINE_BINARY_ASSIGNMENT(bitwise_or_assign, |=)
DEFINE_BINARY_ASSIGNMENT(bitwise_xor_assign, ^=)
DEFINE_BINARY_ASSIGNMENT(divides_assign, /=)
DEFINE_BINARY_ASSIGNMENT(minus_assign, -=)
DEFINE_BINARY_ASSIGNMENT(modulus_assign, %=)
DEFINE_BINARY_ASSIGNMENT(multiplies_assign, *=)
DEFINE_BINARY_ASSIGNMENT(plus_assign, +=)
DEFINE_BINARY_ASSIGNMENT(shift_left_assign, <<=)
DEFINE_BINARY_ASSIGNMENT(shift_right_assign, >>=)

#undef DEFINE_BINARY_ASSIGNMENT
#endif

#ifndef DEFINE_BINARY_RETURN
#define DEFINE_BINARY_RETURN(Op, T) \
	template <typename T1, typename T2> \
	struct BinaryReturn<T1, T2, Op> { \
		typedef T type; \
	};

DEFINE_BINARY_RETURN(equal_to, bool)
DEFINE_BINARY_RETURN(greater, bool)
DEFINE_BINARY_RETURN(greater_equal, bool)
DEFINE_BINARY_RETURN(less, bool)
DEFINE_BINARY_RETURN(less_equal, bool)
DEFINE_BINARY_RETURN(logical_and, bool)
DEFINE_BINARY_RETURN(logical_or, bool)
DEFINE_BINARY_RETURN(not_equal_to, bool)
DEFINE_BINARY_RETURN(shift_left, T1)
DEFINE_BINARY_RETURN(shift_right, T1)

DEFINE_BINARY_RETURN(assign, T1&)
DEFINE_BINARY_RETURN(bitwise_and_assign, T1&)
DEFINE_BINARY_RETURN(bitwise_or_assign, T1&)
DEFINE_BINARY_RETURN(bitwise_xor_assign, T1&)
DEFINE_BINARY_RETURN(divides_assign, T1&)
DEFINE_BINARY_RETURN(minus_assign, T1&)
DEFINE_BINARY_RETURN(modulus_assign, T1&)
DEFINE_BINARY_RETURN(multiplies_assign, T1&)
DEFINE_BINARY_RETURN(plus_assign, T1&)
DEFINE_BINARY_RETURN(shift_left_assign, T1&)
DEFINE_BINARY_RETURN(shift_right_assign, T1&)

#undef DEFINE_BINARY_RETURN
#endif

template<typename T1, typename T2, typename T3, typename Op>
struct TrinaryReturn {
	typedef typename BinaryReturn<T2, T3, Op>::type type;
};

struct where_function {
	template <typename T1, typename T2, typename T3>
	static typename TrinaryReturn<T1, T2, T3, where_function>::type
	apply(const T1& a, const T2& b, const T3& c) {
		return (a)? b : c;
	}
};

} // smt

#endif // _OPERATOR_H
