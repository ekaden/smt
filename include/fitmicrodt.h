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

#ifndef _FITMICRODT_H
#define _FITMICRODT_H

#include <cmath>
#include <limits>

#include "darray.h"
#include "debug.h"
#include "diffenc.h"
#include "logit.h"
#include "meansignal.h"
#include "neldermead.h"
#include "pow.h"
#include "sarray.h"

namespace smt {

//
// If you use this software, please cite:
//   Kaden E, Kruggel F, and Alexander DC: Quantitative Mapping of the Per-
//   Axon Diffusion Coefficients in Brain White Matter. Magnetic Resonance in
//   Medicine, 75:1752–1763, 2016.  http://dx.doi.org/10.1002/mrm.25734
//

template <typename float_t>
class MicroDTFunction {
public:
	MicroDTFunction(const smt::darray<float_t, 1>& y,
			const smt::diffenc<float_t>& dw,
			const float_t& diffmax = 3.05e-3):
				_y(y),
				_dw(dw),
				_diffmax(diffmax),
				_y0(mean(y, dw)) {
	}

	float_t operator()(const smt::sarray<float_t, 2>& x) const {
		const float_t diff1 = smt::expit(x(0), _diffmax);
		const float_t diff2 = smt::expit(x(1), _diffmax);
		float_t fval = 0;
		for(std::size_t ii = 0; ii < _dw.mapping.size(); ++ii) {
			const float_t bvalue = _dw.bvalues(_dw.mapping(ii));
			if(bvalue > float_t(0)) {
				fval += smt::pow2(_y(ii)-_y0*smt::meansignal(bvalue, diff1, diff2));
			}
		}

		return fval;
	}

	smt::sarray<float_t, 2> init() const {
		smt::sarray<float_t, 2> x0;
		x0(0) = smt::logit(2/float_t(3)*_diffmax, _diffmax);
		x0(1) = smt::logit(1/float_t(3)*_diffmax, _diffmax);

		return x0;
	}

	smt::sarray<float_t, 2> init(const smt::sarray<float_t, 2>& x) const {
		smt::sarray<float_t, 2> x0;
		x0(0) = smt::logit(x(0), _diffmax);
		x0(1) = smt::logit(x(1), _diffmax);

		return x0;
	}

	smt::sarray<float_t, 2> trans(const smt::sarray<float_t, 2>& x) const {
		smt::sarray<float_t, 2> y;
		y(0) = smt::expit(x(0), _diffmax);
		y(1) = smt::expit(x(1), _diffmax);

		return y;
	}

	smt::sarray<float_t, 3> trans0(const smt::sarray<float_t, 2>& x) const {
		smt::sarray<float_t, 3> y;
		y(0) = smt::expit(x(0), _diffmax);
		y(1) = smt::expit(x(1), _diffmax);
		y(2) = _y0;

		return y;
	}

	~MicroDTFunction() {
	}

private:
	const smt::darray<float_t, 1> _y;
	const smt::diffenc<float_t> _dw;
	const float_t _diffmax;
	const float_t _y0;

	float_t mean(const smt::darray<float_t, 1>& y, const smt::diffenc<float_t>& dw) const {
		float_t y0 = 0;
		std::size_t n = 0;
		for(std::size_t ii = 0; ii < dw.mapping.size(); ++ii) {
			if(dw.bvalues(dw.mapping(ii)) == float_t(0)) {
				y0 += y(ii);
				++n;
			}
		}
		insist(n > 0);
		y0 /= n;

		return y0;
	}
};

template <typename float_t>
class MicroDT0Function {
public:
	MicroDT0Function(const smt::darray<float_t, 1>& y,
			const smt::diffenc<float_t>& dw,
			const float_t& diffmax = 3.05e-3):
				_y(y),
				_dw(dw),
				_diffmax(diffmax) {
	}

	float_t operator()(const smt::sarray<float_t, 3>& x) const {
		const float_t diff1 = smt::expit(x(0), _diffmax);
		const float_t diff2 = smt::expit(x(1), _diffmax);
		const float_t e0 = std::exp(x(2));
		float_t fval = 0;
		for(std::size_t ii = 0; ii < _dw.mapping.size(); ++ii) {
			const float_t bvalue = _dw.bvalues(_dw.mapping(ii));
			fval += smt::pow2(_y(ii)-e0*smt::meansignal(bvalue, diff1, diff2));
		}

		return fval;
	}

	smt::sarray<float_t, 3> init() const {
		smt::sarray<float_t, 3> x0;
		x0(0) = smt::logit(2/float_t(3)*_diffmax, _diffmax);
		x0(1) = smt::logit(1/float_t(3)*_diffmax, _diffmax);
		x0(2) = std::log(maxsignal());

		return x0;
	}

	smt::sarray<float_t, 3> init(const smt::sarray<float_t, 3>& x) const {
		smt::sarray<float_t, 3> x0;
		x0(0) = smt::logit(x(0), _diffmax);
		x0(1) = smt::logit(x(1), _diffmax);
		x0(2) = std::log(x(2));

		return x0;
	}

	smt::sarray<float_t, 3> trans(const smt::sarray<float_t, 3>& x) const {
		smt::sarray<float_t, 3> y;
		y(0) = smt::expit(x(0), _diffmax);
		y(1) = smt::expit(x(1), _diffmax);
		y(2) = std::exp(x(2));

		return y;
	}

	~MicroDT0Function() {
	}

private:
	const smt::darray<float_t, 1> _y;
	const smt::diffenc<float_t> _dw;
	const float_t _diffmax;

	float_t maxsignal() const {
		float_t y_max = -std::numeric_limits<float_t>::infinity();
		for(std::size_t ii = 0; ii < _y.size(); ++ii) {
			y_max = std::max(y_max, _y(ii));
		}

		return y_max;
	}
};

template <typename float_t>
float_t micromd(const float_t& diff1, const float_t& diff2) {
	return (diff1+float_t(2)*diff2)/float_t(3);
}

template <typename float_t>
float_t microfa(const float_t& diff1, const float_t& diff2) {
	const float_t md = micromd(diff1, diff2);
	const float_t tmp = smt::pow2(diff1)+float_t(2)*smt::pow2(diff2);
	if(tmp > float_t(0)) {
		return std::sqrt(float_t(3)/float_t(2)*(smt::pow2(diff1-md)+float_t(2)*smt::pow2(diff2-md))/tmp);
	} else {
		return 0;
	}
}

template <typename float_t>
smt::sarray<float_t, 3> fitmicrodt(const smt::darray<float_t, 1>& y,
		const smt::diffenc<float_t>& dw,
		const float_t& diffmax = 3.05e-3,
		const bool& b0 = false,
		const float_t& opt_rel = 1000*std::numeric_limits<float_t>::epsilon(),
		const float_t& opt_abs = 10*std::numeric_limits<float_t>::epsilon()) {

	// TODO: Random initialisation?

	if(! b0 && dw.any_zero_bvalue()) {
		MicroDTFunction<float_t> f(y, dw, diffmax);
		smt::sNelderMead<float_t, 2, MicroDTFunction<float_t>> ssolver(f);
		ssolver.init(f.init());
		ssolver.solve(opt_rel, opt_abs);
		smt::sarray<float_t, 3> x = f.trans0(ssolver());
		if(x(0) < x(1)) {
			const float_t tmp = x(0);
			x(0) = x(1);
			x(1) = tmp;
		}

		return x;
	} else {
		MicroDT0Function<float_t> f(y, dw, diffmax);
		smt::sNelderMead<float_t, 3, MicroDT0Function<float_t>> ssolver(f);
		ssolver.init(f.init());
		ssolver.solve(opt_rel, opt_abs);
		smt::sarray<float_t, 3> x = f.trans(ssolver());
		if(x(0) < x(1)) {
			const float_t tmp = x(0);
			x(0) = x(1);
			x(1) = tmp;
		}

		return x;
	}
}

} // smt

#endif // _FITMICRODT_H
