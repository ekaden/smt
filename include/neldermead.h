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

#ifndef _NELDERMEAD_H
#define _NELDERMEAD_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

#include "debug.h"
#include "sarray.h"

namespace smt {

//
// Lagarias JC, Reeds JA, Wright MH and Wright PE: Convergence Properties of the
// Nelder-Mead Simplex Method in Low Dimensions. SIAM Journal of Optimization,
// vol. 9, pp. 112–147, 1998.
//
// Nash JC: Compact Numerical Methods for Computers. Adam Hilger, 1990.
//

template <typename float_t, std::size_t N, typename function_t>
class sNelderMead {
public:
	sNelderMead(const function_t& function):
			_rho(1.0),
			_chi(2.0),
			_gamma(0.5),
			_sigma(0.5),
			_function(function) {
		static_assert(N > 0, "N > 0");
	}

	void init(const smt::sarray<float_t, N>& x) {
		// TODO: Better initialisation?

		const float_t alpha0 = 1.0;
		const float_t alpha1 = 0.05;

		float_t xmin = std::numeric_limits<float_t>::max();
		for(std::size_t ii = 0; ii < N; ++ii) {
			if(x(ii) != 0) {
				xmin = std::min(xmin, std::abs(x(ii)));
			}
		}
		smt::sarray<float_t, N> dx;
		if(xmin == std::numeric_limits<float_t>::max()) {
			for(std::size_t ii = 0; ii < N; ++ii) {
				dx(ii) = alpha0;
			}
		} else {
			for(std::size_t ii = 0; ii < N; ++ii) {
				if(x(ii) == 0) {
					dx(ii) = alpha1*xmin;
				} else {
					dx(ii) = alpha1*x(ii);
				}
			}
		}
		init(x, dx);
	}

	void init(const smt::sarray<float_t, N>& x, const smt::sarray<float_t, N>& dx) {
		for(std::size_t ii = 0; ii < N+1; ++ii) {
			_x(ii) = x;
		}
		for(std::size_t ii = 0; ii < N; ++ii) {
			_x(ii+1)(ii) += dx(ii);
		}
		for(std::size_t ii = 0; ii < N+1; ++ii) {
			_fval(ii) = _function(_x(ii));
		}
		for(std::size_t ii = 0; ii < N+1; ++ii) {
			_idx(ii) = ii;
		}
		sort();
	}

	bool solve(const float_t tol_rel = 100*std::numeric_limits<float_t>::epsilon(),
			const float_t tol_abs = std::numeric_limits<float_t>::epsilon(),
			const std::size_t max_iter = 10000) {
		std::size_t iter = 0;
		std::size_t f_calls = N+1;

		bool converged = false;
		while((! converged) && iter < max_iter) {
			const sarray<float_t, N> x_bar = centroid();

			// reflection
			const smt::sarray<float_t, N> x_r = (float_t(1)+_rho)*x_bar-_rho*_x(_idx(N));
			const float_t fval_r = _function(x_r);
			f_calls += 1;

			if(_fval(_idx(0)) <= fval_r && fval_r < _fval(_idx(N-1))) {
				_x(_idx(N)) = x_r;
				_fval(_idx(N)) = fval_r;
				sortN();
			} else if(fval_r < _fval(_idx(0))) {
				// expansion
				const smt::sarray<float_t, N> x_e = (float_t(1)+_rho*_chi)*x_bar-_rho*_chi*_x(_idx(N));
				const float_t fval_e = _function(x_e);
				f_calls += 1;

				if(fval_e < fval_r) {
					_x(_idx(N)) = x_e;
					_fval(_idx(N)) = fval_e;
					sortN();
				} else {
					_x(_idx(N)) = x_r;
					_fval(_idx(N)) = fval_r;
					sortN();
				}
			} else { // fval_r > _fval(_idx(N-1))
				if(_fval(_idx(N-1)) <= fval_r && fval_r < _fval(_idx(N))) {
					// outside contraction
					const smt::sarray<float_t, N> x_c = (float_t(1)+_rho*_gamma)*x_bar-_rho*_gamma*_x(_idx(N));
					const float_t fval_c = _function(x_c);
					f_calls += 1;

					if(fval_c <= fval_r) {
						_x(_idx(N)) = x_c;
						_fval(_idx(N)) = fval_c;
						sortN();
					} else {
						// shrinkage
						for(std::size_t ii = 1; ii < N+1; ++ii) {
							const smt::sarray<float_t, N> xold = _x(_idx(ii));
							_x(_idx(ii)) = _x(_idx(0))+_sigma*(_x(_idx(ii))-_x(_idx(0)));
							const smt::sarray<float_t, N> xnew = _x(_idx(ii));
							_fval(_idx(ii)) = _function(_x(_idx(ii)));
						}
						f_calls += N;
						sort();
					}
				} else { // fval_r >= _fval(_idx(N))
					// inside contraction
					const smt::sarray<float_t, N> x_c = (float_t(1)-_gamma)*x_bar+_gamma*_x(_idx(N));
					const float_t fval_c = _function(x_c);
					f_calls += 1;

					if(fval_c < _fval(_idx(N))) {
						_x(_idx(N)) = x_c;
						_fval(_idx(N)) = fval_c;
						sortN();
					} else {
						// shrinkage
						for(std::size_t ii = 1; ii < N+1; ++ii) {
							const smt::sarray<float_t, N> xold = _x(_idx(ii));
							_x(_idx(ii)) = _x(_idx(0))+_sigma*(_x(_idx(ii))-_x(_idx(0)));
							const smt::sarray<float_t, N> xnew = _x(_idx(ii));
							_fval(_idx(ii)) = _function(_x(_idx(ii)));
						}
						f_calls += N;
						sort();
					}
				}
			}

			// TODO: Check also if the simplex has changed.
			const smt::sarray<float_t, N> dx = _x(0)-_x(N);
			if(std::abs(_fval(_idx(0))-_fval(_idx(N))) <= std::max(tol_abs, tol_rel*std::abs(_fval(_idx(N))))
					&& smt::normInf(dx) <= std::max(tol_abs, tol_rel*smt::normInf(_x(N)))) {
				converged = true;
			}
			++iter;
		}

		return converged;
	}

	smt::sarray<float_t, N> operator()() const {
		return _x(_idx(0));
	}

	float_t fval() const {
		return _fval(_idx(0));
	}

private:
	const float_t _rho; // reflection coefficient
	const float_t _chi; // expansion coefficient
	const float_t _gamma; // contraction coefficient
	const float_t _sigma; // shrinkage coefficient
	const function_t& _function;

	smt::sarray<smt::sarray<float_t, N>, N+1> _x;
	smt::sarray<float_t, N+1> _fval;
	smt::sarray<std::size_t, N+1> _idx;

	smt::sarray<float_t, N> centroid() const {
		smt::sarray<float_t, N> xbar = 0;
		for(std::size_t ii = 0; ii < N; ++ii) {
			xbar += _x(_idx(ii));
		}
		xbar /= N;

		return xbar;
	}

	void sort() {
		std::stable_sort(_idx.begin(), _idx.end(),
				[&](const std::size_t& ii, const std::size_t& jj) { return _fval(ii) <= _fval(jj); });
	}

	void sortN() {
		const float_t fval_tmp = _fval(_idx(N));
		const std::size_t idx_tmp = _idx(N);
		std::size_t ii = N;
		while(ii > 0 && fval_tmp < _fval(_idx(ii-1))) {
			_idx(ii) = _idx(ii-1);
			--ii;
		}
		_idx(ii) = idx_tmp;
	}
};

} // smt

#endif // _NELDERMEAD_H
