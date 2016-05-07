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

#ifndef _RICIANFIT_H
#define _RICIANFIT_H

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

#include "besseli0.h"
#include "darray.h"
#include "debug.h"
#include "neldermead.h"
#include "pow.h"
#include "sarray.h"

namespace smt {

template <typename float_t>
class RiceLikeFunction {
public:
	RiceLikeFunction(const smt::darray<float_t, 1>& y,
			const float_t& minsignal = 0): _y(maxsignal(y, minsignal)) {
	}

	float_t operator()(const smt::sarray<float_t, 2>& x) const {
		const float_t e0 = std::exp(x(0));
		const float_t sigma = std::exp(x(1));
		float_t fval = 0;
		for(std::size_t ii = 0; ii < _y.size(); ++ii) {
			fval += (float_t(0) < _y(ii))? -std::log(_y(ii)/smt::pow2(sigma))+smt::pow2(_y(ii)-e0)/(2*smt::pow2(sigma))-std::log(smt::besselei0(_y(ii)*e0/smt::pow2(sigma))) : std::log(float_t(0));
		}

		return fval;
	}

	smt::sarray<float_t, 2> init() const {
		smt::sarray<float_t, 2> x0;
		x0(0) = std::log(meansignal());
		x0(1) = std::log(stdsignal(x0(0)));

		return x0;
	}

	smt::sarray<float_t, 2> trans(const smt::sarray<float_t, 2>& x) const {
		smt::sarray<float_t, 2> y;
		y(0) = std::exp(x(0));
		y(1) = std::exp(x(1));

		return y;
	}

	~RiceLikeFunction() {
	}

private:
	const smt::darray<float_t, 1> _y;

	smt::darray<float_t, 1> maxsignal(const smt::darray<float_t, 1>& x, const float_t& minsignal) const {
		smt::darray<float_t, 1> y(x.size());
		for(std::size_t ii = 0; ii < y.size(); ++ii) {
			y(ii) = std::max(x(ii), minsignal);
		}

		return y;
	}

	float_t meansignal() const {
		insist(_y.size() > 0);

		float_t y_mean = 0;
		for(std::size_t ii = 0; ii < _y.size(); ++ii) {
			y_mean += _y(ii);
		}

		return y_mean/_y.size();
	}

	float_t stdsignal(const float_t& mean) const {
		insist(_y.size() > 0);

		float_t y_std = 0;
		for(std::size_t ii = 0; ii < _y.size(); ++ii) {
			y_std += smt::pow2(_y(ii)-mean);
		}

		return std::sqrt(y_std/_y.size());
	}
};

template <typename float_t>
smt::sarray<float_t, 2> ricianfit(const smt::darray<float_t, 1>& y,
		const float_t& minsignal = 0,
		const float_t& opt_rel = 1000*std::numeric_limits<float_t>::epsilon(),
		const float_t& opt_abs = 10*std::numeric_limits<float_t>::epsilon()) {
	RiceLikeFunction<float_t> f(y, minsignal);
	smt::sNelderMead<float_t, 2, RiceLikeFunction<float_t>> solver(f);
	solver.init(f.init());
	solver.solve(opt_rel, opt_abs);
	smt::sarray<float_t, 2> x = f.trans(solver());

	return x;
}

} // smt

#endif // _RICIANFIT_H
