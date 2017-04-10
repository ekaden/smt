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

#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "darray.h"
#include "debug.h"
#include "diffenc.h"
#include "fitmicrodt.h"
#include "fmt.h"
#include "nifti.h"
#include "opts.h"
#include "progress.h"
#include "ricedebias.h"
#include "sarray.h"
#include "version.h"

static const char VERSION[] = R"(fitmicrodt)" " " STR(SMT_VERSION_STRING);

static const char LICENSE[] = R"(
Copyright (c) 2016-2017 Enrico Kaden & University College London
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
)";

static const char USAGE[] = R"(
MICROSCOPIC DIFFUSION TENSOR IMAGING (SPHERICAL MEAN TECHNIQUE)

Copyright (c) 2016-2017 Enrico Kaden & University College London

If you use this software, please cite:
  Kaden E, Kruggel F, and Alexander DC: Quantitative Mapping of the Per-Axon
  Diffusion Coefficients in Brain White Matter. Magnetic Resonance in Medicine,
  75:1752–1763, 2016.  http://dx.doi.org/10.1002/mrm.25734

Usage:
  fitmicrodt [options] <input> <output>
  fitmicrodt (-h | --help)
  fitmicrodt --license
  fitmicrodt --version

Options:
  --bvals <bvals>      Diffusion weighting factors (s/mm²) in FSL format
  --bvecs <bvecs>      Diffusion gradient directions in FSL format
  --grads <grads>      Diffusion gradients (s/mm²) in MRtrix format
  --graddev <graddev>  Diffusion gradient deviation [default: none]
  --mask <mask>        Foreground mask [default: none]
  --rician <rician>    Rician noise [default: none]
  --maxdiff <maxdiff>  Maximum diffusivity (mm²/s) [default: 3.05e-3]
  --b0                 Model-based estimation of zero b-value signal
  -h, --help           Help screen
  --license            License information
  --version            Software version
)";

template <typename float_t>
smt::diffenc<float_t> read_diffenc(std::map<std::string, docopt::value>& args) {
	if(args["--bvals"] && args["--bvecs"] && !args["--grads"]) {
		return smt::diffenc<float_t>(args["--bvals"].asString(), args["--bvecs"].asString());
	} else if(!args["--bvals"] && !args["--bvecs"] && args["--grads"]) {
		return smt::diffenc<float_t>(args["--grads"].asString());
	} else {
		smt::error("Either --bvals <bvals>, --bvecs <bvecs> or --grads <grads> are required.");
		std::exit(EXIT_FAILURE);
	}
}

template <typename float_t>
smt::inifti<float_t, 4> read_graddev(std::map<std::string, docopt::value>& args) {
	if(args["--graddev"] && args["--graddev"].asString() != "none") {
		return smt::inifti<float_t, 4>(args["--graddev"].asString());
	} else {
		return smt::inifti<float_t, 4>();
	}
}

template <typename float_t>
smt::inifti<float_t, 3> read_mask(std::map<std::string, docopt::value>& args) {
	if(args["--mask"] && args["--mask"].asString() != "none") {
		return smt::inifti<float_t, 3>(args["--mask"].asString());
	} else {
		return smt::inifti<float_t, 3>();
	}
}

template <typename float_t>
std::tuple<float_t, smt::inifti<float_t, 3>> read_rician(std::map<std::string, docopt::value>& args) {
	if(args["--rician"] && args["--rician"].asString() != "none") {
		std::istringstream sin(args["--rician"].asString());
		float_t scalar;
		if(! (sin >> scalar)) {
			return std::make_tuple(float_t(0), smt::inifti<float_t, 3>(args["--rician"].asString()));
		} else {
			return std::make_tuple(scalar, smt::inifti<float_t, 3>());
		}
	} else {
		return std::make_tuple(float_t(0), smt::inifti<float_t, 3>());
	}
}

template <typename float_t>
float_t read_maxdiff(std::map<std::string, docopt::value>& args) {
	if(args["--maxdiff"]) {
		std::istringstream sin(args["--maxdiff"].asString());
		float_t maxdiff;
		if(! (sin >> maxdiff)) {
			smt::error("Unable to parse ‘" + args["--maxdiff"].asString() + "’.");
			std::exit(EXIT_FAILURE);
		} else {
			return maxdiff;
		}
	} else {
		return float_t(3.05e-3);
	}
}

template <typename float_t>
smt::sarray<float_t, 3, 3> reshape_graddev(const smt::darray<float_t, 1>& g) {
	smt::assert(g.size(0) == 9);
	smt::sarray<float_t, 3, 3> G;
	G(0, 0) = g(0);
	G(1, 0) = g(1);
	G(2, 0) = g(2);
	G(0, 1) = g(3);
	G(1, 1) = g(4);
	G(2, 1) = g(5);
	G(0, 2) = g(6);
	G(1, 2) = g(7);
	G(2, 2) = g(8);

	return G;
}

int main(int argc, const char** argv) {

	typedef double float_t;

	// Input

	std::map<std::string, docopt::value> args = smt::docopt(USAGE, {argv+1, argv+argc}, true, VERSION);
	if(args["--license"].asBool()) {
		std::cout << LICENSE << std::endl;
		return EXIT_SUCCESS;
	}

	const smt::inifti<float_t, 4> input(args["<input>"].asString());

	const smt::diffenc<float_t> dw = read_diffenc<float_t>(args);
	if(input.size(3) != dw.mapping.size(0)) {
		if(args["--bvals"] && args["--bvecs"] && !args["--grads"]) {
			smt::error("‘" + args["<input>"].asString() + "’ and ‘" + args["--bvals"].asString() + "’ and/or ‘" + args["--bvecs"].asString() + "’ do not match.");
		} else if(!args["--bvals"] && !args["--bvecs"] && args["--grads"]) {
			smt::error("‘" + args["<input>"].asString() + "’ and ‘" + args["--grads"].asString() + "’ do not match.");
		} else {
			smt::error("Either --bvals <bvals>, --bvecs <bvecs> or --grads <grads> are required.");
		}
		return EXIT_FAILURE;
	}

	const smt::inifti<float_t, 4> graddev = read_graddev<float_t>(args);
	if(graddev) {
		if(input.size(0) != graddev.size(0) || input.size(1) != graddev.size(1) || input.size(2) != graddev.size(2)) {
			smt::error("‘" + args["<input>"].asString() + "’ and ‘" + args["--graddev"].asString() + "’ do not match.");
			return EXIT_FAILURE;
		}
		if(graddev.size(3) != 9) {
			smt::error("‘" + args["--graddev"].asString() + "’ does not contain nine volumes.");
			return EXIT_FAILURE;
		}
		if(input.pixsize(0) != graddev.pixsize(0) || input.pixsize(1) != graddev.pixsize(1) || input.pixsize(2) != graddev.pixsize(2)) {
			smt::error("The pixel sizes of ‘" + args["<input>"].asString() + "’ and ‘" + args["--graddev"].asString() + "’ do not match.");
			return EXIT_FAILURE;
		}
		if(! input.has_equal_spatial_coords(graddev)) {
			smt::error("The coordinate systems of ‘" + args["<input>"].asString() + "’ and ‘" + args["--graddev"].asString() + "’ do not match.");
			return EXIT_FAILURE;
		}
	}

	const smt::inifti<float_t, 3> mask = read_mask<float_t>(args);
	if(mask) {
		if(input.size(0) != mask.size(0) || input.size(1) != mask.size(1) || input.size(2) != mask.size(2)) {
			smt::error("‘" + args["<input>"].asString() + "’ and ‘" + args["--mask"].asString() + "’ do not match.");
			return EXIT_FAILURE;
		}
		if(input.pixsize(0) != mask.pixsize(0) || input.pixsize(1) != mask.pixsize(1) || input.pixsize(2) != mask.pixsize(2)) {
			smt::error("The pixel sizes of ‘" + args["<input>"].asString() + "’ and ‘" + args["--mask"].asString() + "’ do not match.");
			return EXIT_FAILURE;
		}
		if(! input.has_equal_spatial_coords(mask)) {
			smt::error("The coordinate systems of ‘" + args["<input>"].asString() + "’ and ‘" + args["--mask"].asString() + "’ do not match.");
			return EXIT_FAILURE;
		}
	}

	const std::tuple<float_t, smt::inifti<float_t, 3>> rician = read_rician<float_t>(args);
	if(std::get<1>(rician)) {
		if(input.size(0) != std::get<1>(rician).size(0) || input.size(1) != std::get<1>(rician).size(1) || input.size(2) != std::get<1>(rician).size(2)) {
			smt::error("‘" + args["<input>"].asString() + "’ and ‘" + args["--rician"].asString() + "’ do not match.");
			return EXIT_FAILURE;
		}
		if(input.pixsize(0) != std::get<1>(rician).pixsize(0) || input.pixsize(1) != std::get<1>(rician).pixsize(1) || input.pixsize(2) != std::get<1>(rician).pixsize(2)) {
			smt::error("The pixel sizes of ‘" + args["<input>"].asString() + "’ and ‘" + args["--rician"].asString() + "’ do not match.");
			return EXIT_FAILURE;
		}
		if(! input.has_equal_spatial_coords(std::get<1>(rician))) {
			smt::error("The coordinate systems of ‘" + args["<input>"].asString() + "’ and ‘" + args["--rician"].asString() + "’ do not match.");
			return EXIT_FAILURE;
		}
	}

	const float_t maxdiff = read_maxdiff<float_t>(args);

	const bool b0 = args["--b0"].asBool();

	const int split = smt::is_format_string(args["<output>"].asString());
	if(split < 0) {
		smt::error("‘" + args["<output>"].asString() + "’ is malformed.");
		std::exit(EXIT_FAILURE);
	}

	// Processing

	smt::onifti<float, 3> output_long = (split > 0)? smt::onifti<float, 3>(smt::format_string(args["<output>"].asString(), "long"), input, input.size(0), input.size(1), input.size(2)) : smt::onifti<float, 3>();
	smt::onifti<float, 3> output_trans = (split > 0)? smt::onifti<float, 3>(smt::format_string(args["<output>"].asString(), "trans"), input, input.size(0), input.size(1), input.size(2)) : smt::onifti<float, 3>();
	smt::onifti<float, 3> output_fa = (split > 0)? smt::onifti<float, 3>(smt::format_string(args["<output>"].asString(), "fa"), input, input.size(0), input.size(1), input.size(2)) : smt::onifti<float, 3>();
	smt::onifti<float, 3> output_fapow3 = (split > 0)? smt::onifti<float, 3>(smt::format_string(args["<output>"].asString(), "fapow3"), input, input.size(0), input.size(1), input.size(2)) : smt::onifti<float, 3>();
	smt::onifti<float, 3> output_md = (split > 0)? smt::onifti<float, 3>(smt::format_string(args["<output>"].asString(), "md"), input, input.size(0), input.size(1), input.size(2)) : smt::onifti<float, 3>();
	smt::onifti<float, 3> output_b0 = (split > 0)? smt::onifti<float, 3>(smt::format_string(args["<output>"].asString(), "b0"), input, input.size(0), input.size(1), input.size(2)) : smt::onifti<float, 3>();
	smt::onifti<float, 4> output = (split > 0)? smt::onifti<float, 4>() : smt::onifti<float, 4>(smt::format_string(args["<output>"].asString()), input, input.size(0), input.size(1), input.size(2), 6);

	if(split > 0) {
		output_long.cal(0, maxdiff);
		output_trans.cal(0, maxdiff);
		output_fa.cal(0, 1);
		output_fapow3.cal(0, 1);
		output_md.cal(0, maxdiff);
	}

	const std::size_t input_size_0 = input.size(0);
	const std::size_t input_size_1 = input.size(1);
	const std::size_t input_size_2 = input.size(2);
	smt::progress p{input_size_0*input_size_1*input_size_2, "fitmicrodt"};
#pragma omp parallel for schedule(dynamic, 10) collapse(3)
	for(std::size_t kk = 0; kk < input_size_2; ++kk) {
		for(std::size_t jj = 0; jj < input_size_1; ++jj) {
			for(std::size_t ii = 0; ii < input_size_0; ++ii) {
				if((! mask) || mask(ii, jj, kk) > 0) {
					smt::darray<float_t, 1> input_tmp = input(ii, jj, kk, smt::slice(0, input.size(3)));
					if(std::get<1>(rician)) {
						for(std::size_t ll = 0; ll < input.size(3); ++ll) {
							input_tmp(ll) = smt::ricedebias(input_tmp(ll), std::get<1>(rician)(ii, jj, kk));
						}
					} else {
						if(std::get<0>(rician) > float_t(0)) {
							for(std::size_t ll = 0; ll < input.size(3); ++ll) {
								input_tmp(ll) = smt::ricedebias(input_tmp(ll), std::get<0>(rician));
							}
						}
					}

					const smt::diffenc<float_t> dw_tmp = (graddev)?
							smt::diffenc<float_t>(dw, reshape_graddev(graddev(ii, jj, kk, smt::slice(0, 9)))) : dw;

					const smt::sarray<float_t, 3> fit = smt::fitmicrodt(input_tmp, dw_tmp, maxdiff, b0);
					if(split > 0) {
						output_long(ii, jj, kk) = fit(0);
						output_trans(ii, jj, kk) = fit(1);
						output_fa(ii, jj, kk) = smt::microfa(fit(0), fit(1));
						output_fapow3(ii, jj, kk) = std::pow(smt::microfa(fit(0), fit(1)), 3);
						output_md(ii, jj, kk) = smt::micromd(fit(0), fit(1));
						output_b0(ii, jj, kk) = fit(2);
					} else {
						output(ii, jj, kk, 0) = fit(0);
						output(ii, jj, kk, 1) = fit(1);
						output(ii, jj, kk, 2) = smt::microfa(fit(0), fit(1));
						output(ii, jj, kk, 3) = std::pow(smt::microfa(fit(0), fit(1)), 3);
						output(ii, jj, kk, 4) = smt::micromd(fit(0), fit(1));
						output(ii, jj, kk, 5) = fit(2);
					}
				} else {
					if(split > 0) {
						output_long(ii, jj, kk) = 0;
						output_trans(ii, jj, kk) = 0;
						output_fa(ii, jj, kk) = 0;
						output_fapow3(ii, jj, kk) = 0;
						output_md(ii, jj, kk) = 0;
						output_b0(ii, jj, kk) = 0;
					} else {
						output(ii, jj, kk, 0) = 0;
						output(ii, jj, kk, 1) = 0;
						output(ii, jj, kk, 2) = 0;
						output(ii, jj, kk, 3) = 0;
						output(ii, jj, kk, 4) = 0;
						output(ii, jj, kk, 5) = 0;
					}
				}
				++p;
			}
		}
	}

	return EXIT_SUCCESS;
}
