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

#ifndef _DIFFENC_H
#define _DIFFENC_H

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "darray.h"
#include "sarray.h"

namespace smt {

template <typename float_t>
class diffenc_t {
public:
	smt::darray<float_t, 1> bvalues;
	smt::darray<smt::sarray<float_t, 3>, 1> gradients;
	smt::darray<std::size_t, 1> mapping;

	diffenc_t(const std::string& filename_bvals, const std::string& filename_bvecs):
			bvalues(read_bvals(filename_bvals)),
			gradients(read_bvecs(filename_bvecs)),
			mapping(read_bmap(filename_bvals, filename_bvecs)) {

// TODO: Explicit normalisation?
//		for(std::size_t ii = 0; ii < bvalues.size(0); ++ii) {
//			const float_t norm_tmp_pow2 = smt::dot(gradients(ii), gradients(ii));
//			bvalues(ii) *= norm_tmp_pow2;
//			if(norm_tmp_pow2 == float_t(0)) {
//				gradients(ii) = 0;
//			} else {
//				gradients(ii) /= std::sqrt(norm_tmp_pow2);
//			}
//		}
	}

	void corrdiffenc(smt::sarray<float_t, 3, 3> graddev) {
		graddev(0, 0) += float_t(1);
		graddev(1, 1) += float_t(1);
		graddev(2, 2) += float_t(1);

		for(std::size_t ii = 0; ii < bvalues.size(0); ++ii) {
			const smt::sarray<float_t, 3> tmp = smt::gemv(graddev, gradients(ii));
			const float_t norm_tmp_pow2 = smt::dot(tmp, tmp);
			bvalues(ii) *= norm_tmp_pow2;
			if(norm_tmp_pow2 == float_t(0)) {
				gradients(ii) = 0;
			} else {
				gradients(ii) = tmp/std::sqrt(norm_tmp_pow2);
			}
		}
	}

	bool anyZeroBValue() const {
		for(std::size_t ii = 0; ii < mapping.size(0); ++ii) {
			if(bvalues(mapping(ii)) == float_t(0)) {
				return true;
			}
		}

		return false;
	}

private:
	smt::darray<float_t, 1> read_bvals(const std::string& filename) const {
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
			std::cerr << "ERROR: Unable to read '" << filename << "'." << std::endl;
			std::exit(EXIT_FAILURE);
		}
		smt::darray<float_t, 1> bvals(buf.size());
		for(std::size_t ii = 0; ii < bvals.size(0); ++ii) {
			bvals(ii) = buf[ii];
		}

		return bvals;
	}

	smt::darray<smt::sarray<float_t, 3>, 1> read_bvecs(const std::string& filename) const {
		std::ifstream fin(filename.c_str());
		std::deque<float_t> buf[3];
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
			std::cerr << "ERROR: Unable to read '" << filename << "'." << std::endl;
			std::exit(EXIT_FAILURE);
		}
		smt::darray<smt::sarray<float_t, 3>, 1> bvecs(std::min(buf[0].size(), std::min(buf[1].size(), buf[2].size())));
		for(std::size_t ii = 0; ii < bvecs.size(0); ++ii) {
			bvecs(ii)(0) = buf[0][ii];
			bvecs(ii)(1) = buf[1][ii];
			bvecs(ii)(2) = buf[2][ii];
		}

		return bvecs;
	}

	smt::darray<std::size_t, 1> read_bmap(const std::string& filename_bvals, const std::string& filename_bvecs) const {
		if(bvalues.size(0) != gradients.size(0)) {
			std::cerr << "ERROR: '" << filename_bvals << "' and '" << filename_bvecs << "' do not match." << std::endl;
			std::exit(EXIT_FAILURE);
		}

		smt::darray<std::size_t, 1> bmap(bvalues.size(0));
		for(std::size_t ii = 0; ii < bmap.size(0); ++ii) {
			bmap(ii) = ii;
		}

		return bmap;
	}
};

} // smt

#endif // _DIFFENC_H
