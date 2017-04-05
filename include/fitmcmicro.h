//
// Copyright (c) 2016-2017 Enrico Kaden & University College London
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

#ifndef _FITMCMICRO_H
#define _FITMCMICRO_H

#include <limits>

#include "darray.h"
#include "debug.h"
#include "diffenc.h"
#include "logit.h"
#include "meansignal.h"
#include "neldermead.h"
#include "pow.h"
#include "project.h"
#include "sarray.h"

namespace smt {

//
// If you use this software, please cite:
//   Kaden E, Kelm ND, Carson RP, Does MD, and Alexander DC: Multi-compartment
//   microscopic diffusion anisotropy imaging brought into clinical practice.
//   In: Proceedings of the 24th Annual Meeting of the International Society for
//   Magnetic Resonance in Medicine, p. 1078, 2016.
//

template <typename float_t>
class McMicroFunction {
public:
	McMicroFunction(const smt::darray<float_t, 1>& y,
			const smt::diffenc<float_t>& dw,
			const float_t& diffmax = 3.05e-3):
				_y(y),
				_dw(dw),
				_intramax(1),
				_diffmax(diffmax),
				_y0(mean(y, dw)) {
	}

	float_t operator()(const smt::sarray<float_t, 2>& x) const {
		const float_t intra = smt::expit(x(0), _intramax);
		const float_t diff = smt::expit(x(1), _diffmax);
		float_t fval = 0;
		for(std::size_t ii = 0; ii < _dw.mapping.size(); ++ii) {
			const float_t bvalue = _dw.bvalues(_dw.mapping(ii));
			if(bvalue > float_t(0)) {
				fval += smt::pow2(_y(ii)-_y0*(intra*smt::meansignal(bvalue, diff, float_t(0))+(float_t(1)-intra)*smt::meansignal(bvalue, diff, tortuosity(intra)*diff)));
			}
		}

		return fval;
	}

	smt::sarray<float_t, 2> init() const {
		smt::sarray<float_t, 2> x0;
		x0(0) = smt::logit(float_t(0.5)*_intramax, _intramax);
		x0(1) = smt::logit(float_t(0.5)*_diffmax, _diffmax);

		return x0;
	}

	smt::sarray<float_t, 2> init(const smt::sarray<float_t, 2>& x) const {
		smt::sarray<float_t, 2> x0;
		x0(0) = smt::logit(x(0), _intramax);
		x0(1) = smt::logit(x(1), _diffmax);

		return x0;
	}

	smt::sarray<float_t, 2> trans(const smt::sarray<float_t, 2>& x) const {
		smt::sarray<float_t, 2> y;
		y(0) = smt::expit(x(0), _intramax);
		y(1) = smt::expit(x(1), _diffmax);

		return y;
	}

	smt::sarray<float_t, 3> trans0(const smt::sarray<float_t, 2>& x) const {
		smt::sarray<float_t, 3> y;
		y(0) = smt::expit(x(0), _intramax);
		y(1) = smt::expit(x(1), _diffmax);
		y(2) = _y0;

		return y;
	}

	~McMicroFunction() {
	}

private:
	const smt::darray<float_t, 1> _y;
	const smt::diffenc<float_t> _dw;
	const float_t _intramax;
	const float_t _diffmax;
	const float_t _y0;

	float_t tortuosity(const float_t& intra) const {
		return float_t(1)-smt::project(intra, float_t(0), _intramax);
	}

	float_t mean(const smt::darray<float_t, 1>& y, const smt::diffenc<float_t>& dw) const {
		float_t y0 = 0;
		std::size_t n = 0;
		for(std::size_t ii = 0; ii < dw.mapping.size(); ++ii) {
			if(dw.bvalues(dw.mapping(ii)) == float_t(0)) {
				y0 += y(ii);
				++n;
			}
		}
		smt::assert(n > 0);
		y0 /= n;

		return y0;
	}
};

template <typename float_t>
class McMicro0Function {
public:
	McMicro0Function(const smt::darray<float_t, 1>& y,
			const smt::diffenc<float_t>& dw,
			const float_t& diffmax = 3.05e-3):
				_y(y),
				_dw(dw),
				_intramax(1),
				_diffmax(diffmax) {
	}

	float_t operator()(const smt::sarray<float_t, 3>& x) const {
		const float_t intra = smt::expit(x(0), _intramax);
		const float_t diff = smt::expit(x(1), _diffmax);
		const float_t e0 = std::exp(x(2));
		float_t fval = 0;
		for(std::size_t ii = 0; ii < _dw.mapping.size(); ++ii) {
			const float_t bvalue = _dw.bvalues(_dw.mapping(ii));
			fval += smt::pow2(_y(ii)-e0*(intra*smt::meansignal(bvalue, diff, float_t(0))+(float_t(1)-intra)*smt::meansignal(bvalue, diff, tortuosity(intra)*diff)));
		}

		return fval;
	}

	smt::sarray<float_t, 3> init() const {
		smt::sarray<float_t, 3> x0;
		x0(0) = smt::logit(float_t(0.5)*_intramax, _intramax);
		x0(1) = smt::logit(float_t(0.5)*_diffmax, _diffmax);
		x0(2) = std::log(maxsignal());

		return x0;
	}

	smt::sarray<float_t, 3> init(const smt::sarray<float_t, 3>& x) const {
		smt::sarray<float_t, 3> x0;
		x0(0) = smt::logit(x(0), _intramax);
		x0(1) = smt::logit(x(1), _diffmax);
		x0(2) = std::log(x(2));

		return x0;
	}

	smt::sarray<float_t, 3> trans(const smt::sarray<float_t, 3>& x) const {
		smt::sarray<float_t, 3> y;
		y(0) = smt::expit(x(0), _intramax);
		y(1) = smt::expit(x(1), _diffmax);
		y(2) = std::exp(x(2));

		return y;
	}

	~McMicro0Function() {
	}

private:
	const smt::darray<float_t, 1> _y;
	const smt::diffenc<float_t> _dw;
	const float_t _intramax;
	const float_t _diffmax;

	float_t tortuosity(const float_t& intra) const {
		return float_t(1)-smt::project(intra, float_t(0), _intramax);
	}

	float_t maxsignal() const {
		float_t y_max = -std::numeric_limits<float_t>::infinity();
		for(std::size_t ii = 0; ii < _y.size(); ++ii) {
			y_max = std::max(y_max, _y(ii));
		}

		return y_max;
	}
};

template <typename float_t>
smt::sarray<float_t, 3> fitmcmicro(const smt::darray<float_t, 1>& y,
		const smt::diffenc<float_t>& dw,
		const float_t& diffmax = 3.05e-3,
		const bool& b0 = false,
		const float_t& opt_rel = 1000*std::numeric_limits<float_t>::epsilon(),
		const float_t& opt_abs = 10*std::numeric_limits<float_t>::epsilon()) {

	// TODO: Random initialisation?

	if(! b0 && dw.any_zero_bvalue()) {
		McMicroFunction<float_t> f(y, dw, diffmax);
		smt::sNelderMead<float_t, 2, McMicroFunction<float_t>> ssolver(f);
		ssolver.init(f.init());
		ssolver.solve(opt_rel, opt_abs);
		const smt::sarray<float_t, 3> x = f.trans0(ssolver());

		return x;
	} else {
		McMicro0Function<float_t> f(y, dw, diffmax);
		smt::sNelderMead<float_t, 3, McMicro0Function<float_t>> ssolver(f);
		ssolver.init(f.init());
		ssolver.solve(opt_rel, opt_abs);
		const smt::sarray<float_t, 3> x = f.trans(ssolver());

		return x;
	}
}

} // smt

#endif // _FITMCMICRO_H
