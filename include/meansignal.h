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

#ifndef _MEANSIGNAL_H
#define _MEANSIGNAL_H

#include <cmath>

#include "debug.h"

namespace smt {

// TODO: The order of lambda1 and lambda2 is irrelevant. We assume that the
// greater value indicates the longitudinal microscopic diffusivity, which may
// give rise to a discontinuity in the derivatives.

// TODO: Efficient computation of erf

template<typename float_t>
float_t meansignal(const float_t bvalue, const float_t lambda1, const float_t lambda2) {
	if(lambda1 > lambda2) {
		if(bvalue == 0) {
			return float_t(1);
		} else {
			const float_t tmp = std::sqrt(bvalue*(lambda1-lambda2));
			return std::sqrt(float_t(M_PI))*std::exp(-bvalue*lambda2)*erf(tmp)/(float_t(2)*tmp);
		}
	} else if(lambda1 == lambda2) {
		return std::exp(-bvalue*lambda1);
	} else if(lambda1 < lambda2) {
		return meansignal(bvalue, lambda2, lambda1);
	} else {
		insist(false);
	}
}

} // smt

#endif // _MEANSIGNAL_H
