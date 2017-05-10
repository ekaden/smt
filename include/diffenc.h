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

#ifndef _DIFFENC_H
#define _DIFFENC_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>
#include <tuple>

#include "darray.h"
#include "debug.h"
#include "sarray.h"

namespace smt {

template <typename float_t>
class diffenc {
public:
	const smt::darray<float_t, 1> bvalues;
	const smt::darray<smt::sarray<float_t, 3>, 1> gradients;
	const smt::darray<std::size_t, 1> mapping;

	diffenc() {}

	diffenc(const std::string& filename_bvals, const std::string& filename_bvecs):
		diffenc(diffenc_fsl(filename_bvals, filename_bvecs)) {

		if(! has_nonnegative_bvalues()) {
			smt::error("‘" + filename_bvals + "’ has diffusion weighting factors which are not non-negative.");
			std::exit(EXIT_FAILURE);
		}
		if(! has_normalised_gradients(100*std::numeric_limits<float>::epsilon())) {
			smt::error("‘" + filename_bvecs + "’ has diffusion gradient directions which are not normalised.");
			std::exit(EXIT_FAILURE);
		}
	}

	diffenc(const std::string& filename):
		diffenc(diffenc_mrtrix(filename)) {

		if(! has_nonnegative_bvalues()) {
			smt::error("‘" + filename + "’ has diffusion weighting factors which are not non-negative.");
			std::exit(EXIT_FAILURE);
		}
		if(! has_normalised_gradients(100*std::numeric_limits<float>::epsilon())) {
			smt::error("‘" + filename + "’ has diffusion gradient directions which are not normalised.");
			std::exit(EXIT_FAILURE);
		}
	}

	diffenc(const diffenc& rhs, const smt::sarray<float_t, 3, 3>& graddev):
		diffenc(diffenc_graddev(rhs, graddev)) {}

	explicit operator bool() const {
		return (mapping)? true : false;
	}

	bool any_zero_bvalue() const {
		return std::any_of(std::begin(mapping), std::end(mapping), [&](const std::size_t& idx) {
			return bvalues(idx) == float_t(0);
		});
	}

private:
	diffenc(const std::tuple<smt::darray<float_t, 1>, smt::darray<smt::sarray<float_t, 3>, 1>, smt::darray<std::size_t, 1>>& rhs):
		bvalues(std::get<0>(rhs)),
		gradients(std::get<1>(rhs)),
		mapping(std::get<2>(rhs)) {}

	std::tuple<smt::darray<float_t, 1>, smt::darray<smt::sarray<float_t, 3>, 1>, smt::darray<std::size_t, 1>> diffenc_fsl(
			const std::string& filename_bvals, const std::string& filename_bvecs) const {
		const std::deque<float_t> buf_bvals(read_bvals_fsl(filename_bvals));
		smt::darray<float_t, 1> bvalues_(buf_bvals.size());
		for(std::size_t ii = 0; ii < bvalues_.size(0); ++ii) {
			bvalues_(ii) = buf_bvals[ii];
		}
		const std::array<std::deque<float_t>, 3> buf_bvecs(read_bvecs_fsl(filename_bvecs));
		smt::darray<smt::sarray<float_t, 3>, 1> gradients_(std::min(buf_bvecs[0].size(), std::min(buf_bvecs[1].size(), buf_bvecs[2].size())));
		for(std::size_t ii = 0; ii < gradients_.size(0); ++ii) {
			gradients_(ii)(0) = buf_bvecs[0][ii];
			gradients_(ii)(1) = buf_bvecs[1][ii];
			gradients_(ii)(2) = buf_bvecs[2][ii];
		}
		if(bvalues_.size(0) != gradients_.size(0)) {
			smt::error("‘" + filename_bvals + "’ and ‘" + filename_bvecs + "’ do not match.");
			std::exit(EXIT_FAILURE);
		}
		smt::darray<std::size_t, 1> mapping_(bvalues_.size(0));
		std::iota(std::begin(mapping_), std::end(mapping_), 0);

		return std::make_tuple(bvalues_, gradients_, mapping_);
	}

	std::tuple<smt::darray<float_t, 1>, smt::darray<smt::sarray<float_t, 3>, 1>, smt::darray<std::size_t, 1>> diffenc_mrtrix(
			const std::string& filename) const {
		const std::deque<smt::sarray<float_t, 4>> buf(read_grads_mrtrix(filename));
		smt::darray<float_t, 1> bvalues_(buf.size());
		for(std::size_t ii = 0; ii < bvalues_.size(0); ++ii) {
			bvalues_(ii) = buf[ii][3];
		}
		smt::darray<smt::sarray<float_t, 3>, 1> gradients_(buf.size());
		for(std::size_t ii = 0; ii < gradients_.size(0); ++ii) {
			gradients_(ii)(0) = buf[ii][0];
			gradients_(ii)(1) = buf[ii][1];
			gradients_(ii)(2) = buf[ii][2];
		}
		smt::darray<std::size_t, 1> mapping_(bvalues_.size(0));
		std::iota(std::begin(mapping_), std::end(mapping_), 0);

		return std::make_tuple(bvalues_, gradients_, mapping_);
	}

	std::tuple<smt::darray<float_t, 1>, smt::darray<smt::sarray<float_t, 3>, 1>, smt::darray<std::size_t, 1>> diffenc_graddev(
			const diffenc& rhs, const smt::sarray<float_t, 3, 3>& graddev) const {
		const smt::sarray<float_t, 3, 3> id(smt::eye<float_t, 3, 3>());
		smt::darray<float_t, 1> bvalues_(rhs.bvalues);
		smt::darray<smt::sarray<float_t, 3>, 1> gradients_(rhs.gradients);
		for(std::size_t ii = 0; ii < bvalues_.size(0); ++ii) {
			const smt::sarray<float_t, 3> tmp = smt::gemv(smt::sarray<float_t, 3, 3>(id+graddev), gradients_(ii));
			const float_t norm_tmp_pow2 = smt::dot(tmp, tmp);
			bvalues_(ii) *= norm_tmp_pow2;
			if(norm_tmp_pow2 == float_t(0)) {
				gradients_(ii) = float_t(0);
			} else {
				gradients_(ii) = tmp/std::sqrt(norm_tmp_pow2);
			}
		}
		smt::darray<std::size_t, 1> mapping_(rhs.mapping);

		return std::make_tuple(bvalues_, gradients_, mapping_);
	}

	std::deque<float_t> read_bvals_fsl(const std::string& filename) const {
		std::ifstream fin(filename.c_str());
		std::deque<float_t> buf;
		if(fin.good()) {
			std::size_t line = 0;
			std::string str;
			while(line < 1 && std::getline(fin, str)) {
				std::istringstream sin(str);
				float_t tmp;
				while(sin >> tmp) {
					buf.push_back(tmp);
				}
				++line;
			}
			fin.close();
		} else {
			smt::error("Unable to read ‘" + filename + "’.");
			std::exit(EXIT_FAILURE);
		}

		return buf;
	}

	std::array<std::deque<float_t>, 3> read_bvecs_fsl(const std::string& filename) const {
		std::ifstream fin(filename.c_str());
		std::array<std::deque<float_t>, 3> buf;
		if(fin.good()) {
			std::size_t line = 0;
			std::string str;
			while(line < 3 && std::getline(fin, str)) {
				std::istringstream sin(str);
				float_t tmp;
				while(sin >> tmp) {
					buf[line].push_back(tmp);
				}
				++line;
			}
			fin.close();
		} else {
			smt::error("Unable to read ‘" + filename + "’.");
			std::exit(EXIT_FAILURE);
		}

		return buf;
	}

	std::deque<smt::sarray<float_t, 4>> read_grads_mrtrix(const std::string& filename) const {
		std::ifstream fin(filename.c_str());
		std::deque<smt::sarray<float_t, 4>> buf;
		if(fin.good()) {
			std::string str;
			while(std::getline(fin, str)) {
				std::istringstream sin(str);
				smt::sarray<float_t, 4> tmp;
				if(sin >> tmp[0]
						&& sin.ignore(std::numeric_limits<std::streamsize>::max(), ',')
						&& sin >> tmp[1]
						&& sin.ignore(std::numeric_limits<std::streamsize>::max(), ',')
						&& sin >> tmp[2]
						&& sin.ignore(std::numeric_limits<std::streamsize>::max(), ',')
						&& sin >> tmp[3]) {
					buf.push_back(tmp);
				}
			}
			fin.close();
		} else {
			smt::error("Unable to read ‘" + filename + "’.");
			std::exit(EXIT_FAILURE);
		}

		return buf;
	}

	bool has_nonnegative_bvalues() const {
		return std::all_of(std::begin(mapping), std::end(mapping), [&](const std::size_t& idx) {
			return bvalues(idx) >= float_t(0);
		});
	}

	bool has_normalised_gradients(const float_t& tol) const {
		return std::all_of(std::begin(mapping), std::end(mapping), [&](const std::size_t& idx) {
			const float_t norm_tmp_pow2 = smt::dot(gradients(idx), gradients(idx));
			return norm_tmp_pow2 == float_t(0) || (float_t(1)-tol <= norm_tmp_pow2 && norm_tmp_pow2 <= float_t(1)+tol);
		});
	}
};

} // smt

#endif // _DIFFENC_H
