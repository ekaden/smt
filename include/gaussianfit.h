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

#ifndef _GAUSSIANFIT_H
#define _GAUSSIANFIT_H

#include <cmath>
#include <cstdlib>

#include "darray.h"
#include "pow.h"
#include "sarray.h"

namespace smt {

template <typename float_t>
smt::sarray<float_t, 2> gaussianfit(const smt::darray<float_t, 1>& y) {

	// TODO: Unbiased estimation of standard deviation?

	float_t y_mean = 0;
	for(std::size_t ii = 0; ii < y.size(); ++ii) {
		y_mean += y(ii);
	}
	if(y.size() > 0) {
		y_mean /= y.size();
	}

	float_t y_std = 0;
	for(std::size_t ii = 0; ii < y.size(); ++ii) {
		y_std += smt::pow2(y(ii)-y_mean);
	}
	if(y.size() > 1) {
		y_std /= y.size()-1;
	}
	y_std = std::sqrt(y_std);

	return {y_mean, y_std};
}

} // smt

#endif // _GAUSSIANFIT_H
