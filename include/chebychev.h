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

#ifndef _CHEBYCHEV_H
#define _CHEBYCHEV_H

#include <array>
#include <cstddef>

namespace smt {

template <typename float_t, std::size_t N>
float_t chebeval(float_t x, const std::array<float_t, N>& coeff) {
	static_assert(N > 0, "N > 0");

	x *= float_t(2);

	float_t tmp0 = 0;
	float_t tmp1 = 0;
	float_t tmp2 = coeff[0];

	for(std::size_t ii = 1; ii < N; ++ii) {
		tmp0 = tmp1;
		tmp1 = tmp2;
		tmp2 = x*tmp1-tmp0+coeff[ii];
	}

	return float_t(0.5)*(tmp2-tmp0);
}

} // smt

#endif // _CHEBYCHEV_H
