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

#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "docopt.h"

#include "darray.h"
#include "diffenc.h"
#include "fitmicrodt.h"
#include "nifti.h"
#include "ricedebias.h"
#include "sarray.h"
#include "version.h"

static const char VERSION[] = R"(fitmicrodt )" STR(SMT_VERSION_STRING);

static const char LICENSE[] = R"(
Copyright (c) 2016 Enrico Kaden & University College London
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

Copyright (c) 2016 Enrico Kaden & University College London

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
		std::cerr << "ERROR: Either --bvals <bvals>, --bvecs <bvecs> or --grads <grads> are required." << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

template <typename float_t>
smt::sarray<float_t, 3, 3> reshape_graddev(const smt::darray<float_t, 1>& g) {
	insist(g.size(0) == 9);
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

	std::map<std::string, docopt::value> args = docopt::docopt(USAGE, {argv+1, argv+argc}, true, VERSION);
	if(args["--license"].asBool()) {
		std::cout << LICENSE << std::endl;
		return EXIT_SUCCESS;
	}

	smt::nifti<float_t, 4> input(args["<input>"].asString(), true);

	if(nifti_validfilename(args["<output>"].asString().c_str()) == 0) {
		std::cerr << "ERROR: '" << args["<output>"].asString().c_str() << "' is not a valid NIfTI-1 file name." << std::endl;
		return EXIT_FAILURE;
	}

	const smt::diffenc<float_t> dw = read_diffenc<float_t>(args);
	if(input.size(3) != dw.mapping.size(0)) {
		if(args["--bvals"] && args["--bvecs"] && !args["--grads"]) {
			std::cerr << "ERROR: '" << args["<input>"].asString() << "' and '" << args["--bvals"].asString() << "' and/or '" << args["--bvecs"].asString() << "' do not match." << std::endl;
		} else if(!args["--bvals"] && !args["--bvecs"] && args["--grads"]) {
			std::cerr << "ERROR: '" << args["<input>"].asString() << "' and '" << args["--grads"].asString() << "' do not match." << std::endl;
		} else {
			std::cerr << "ERROR: Either --bvals <bvals>, --bvecs <bvecs> or --grads <grads> are required." << std::endl;
		}
		return EXIT_FAILURE;
	}

	smt::nifti<float_t, 4> graddev;
	if(args["--graddev"] && args["--graddev"].asString() != "none") {
		graddev.read(args["--graddev"].asString(), true);
		if(input.size(0) != graddev.size(0) || input.size(1) != graddev.size(1) || input.size(2) != graddev.size(2)) {
			std::cerr << "ERROR: '" << args["<input>"].asString() << "' and '" << args["--graddev"].asString() << "' do not match." << std::endl;
			return EXIT_FAILURE;
		}
		if(graddev.size(3) != 9) {
			std::cerr << "ERROR: '" << args["--graddev"].asString() << "' does not contain nine volumes." << std::endl;
			return EXIT_FAILURE;
		}
	}

	smt::nifti<float_t, 3> mask;
	if(args["--mask"] && args["--mask"].asString() != "none") {
		mask.read(args["--mask"].asString(), true);
		if(input.size(0) != mask.size(0) || input.size(1) != mask.size(1) || input.size(2) != mask.size(2)) {
			std::cerr << "ERROR: '" << args["<input>"].asString() << "' and '" << args["--mask"].asString() << "' do not match." << std::endl;
			return EXIT_FAILURE;
		}
	}

	float_t rician_scalar = 0;
	smt::nifti<float_t, 3> rician_volume;
	if(args["--rician"] && args["--rician"].asString() != "none") {
		std::istringstream sin(args["--rician"].asString());
		if(! (sin >> rician_scalar)) {
			rician_volume.read(args["--rician"].asString(), true);
			if(input.size(0) != rician_volume.size(0) || input.size(1) != rician_volume.size(1) || input.size(2) != rician_volume.size(2)) {
				std::cerr << "ERROR: '" << args["<input>"].asString() << "' and '" << args["--rician"].asString() << "' do not match." << std::endl;
				return EXIT_FAILURE;
			}
		}
	}

	float_t maxdiff = 3.05e-3;
	if(args["--maxdiff"]) {
		std::istringstream sin(args["--maxdiff"].asString());
		if(! (sin >> maxdiff)) {
			std::cerr << "ERROR: Unable to parse '" << args["--maxdiff"].asString() << "'." << std::endl;
			return EXIT_FAILURE;
		}
	}

	bool b0 = args["--b0"].asBool();

	// Processing

	smt::darray<float, 4> output(input.size(0), input.size(1), input.size(2), 5);
	const std::size_t output_size_0 = output.size(0);
	const std::size_t output_size_1 = output.size(1);
	const std::size_t output_size_2 = output.size(2);
	#pragma omp parallel for schedule(dynamic, 10) collapse(3)
	for(std::size_t ii = 0; ii < output_size_0; ++ii) {
		for(std::size_t jj = 0; jj < output_size_1; ++jj) {
			for(std::size_t kk = 0; kk < output_size_2; ++kk) {
				if((! mask) || mask(ii, jj, kk) > 0) {
					smt::darray<float_t, 1> input_tmp(input.size(3));
					for(std::size_t ll = 0; ll < input.size(3); ++ll) {
						input_tmp(ll) = input(ii, jj, kk, ll);
					}
					if(rician_volume) {
						for(std::size_t ll = 0; ll < input.size(3); ++ll) {
							input_tmp(ll) = smt::ricedebias(input_tmp(ll), rician_volume(ii, jj, kk));
						}
					} else {
						if(rician_scalar > float_t(0)) {
							for(std::size_t ll = 0; ll < input.size(3); ++ll) {
								input_tmp(ll) = smt::ricedebias(input_tmp(ll), rician_scalar);
							}
						}
					}

					const smt::diffenc<float_t> dw_tmp = (graddev)?
							smt::diffenc<float_t>(dw, reshape_graddev(graddev(ii, jj, kk, smt::slice(0, 9)))) : dw;

					smt::sarray<float_t, 3> fitmicrodt_tmp = smt::fitmicrodt(input_tmp, dw_tmp, maxdiff, b0);
					output.colmaj(ii, jj, kk, 0) = fitmicrodt_tmp(0);
					output.colmaj(ii, jj, kk, 1) = fitmicrodt_tmp(1);
					output.colmaj(ii, jj, kk, 2) = smt::microfa(fitmicrodt_tmp(0), fitmicrodt_tmp(1));
					output.colmaj(ii, jj, kk, 3) = smt::micromd(fitmicrodt_tmp(0), fitmicrodt_tmp(1));
					output.colmaj(ii, jj, kk, 4) = fitmicrodt_tmp(2);
				} else {
					output.colmaj(ii, jj, kk, 0) = 0;
					output.colmaj(ii, jj, kk, 1) = 0;
					output.colmaj(ii, jj, kk, 2) = 0;
					output.colmaj(ii, jj, kk, 3) = 0;
					output.colmaj(ii, jj, kk, 4) = 0;
				}
			}
		}
	}

	// Output

	// TODO: Separate NIfTI-1 output (e.g. <output>_diff_long.nii, <output>_diff_trans.nii, <output>_fa.nii, <output>_md.nii, <output>_b0.ni).
	smt::nifti<float, 4> output_nii;
	output_nii.image = nifti_copy_nim_info(input.image);
	nifti_set_filenames(output_nii.image, args["<output>"].asString().c_str(), 0, 0);
	output_nii.image->nt = 5;
	output_nii.image->dim[4] = 5;
	output_nii.image->nvox = output_nii.image->nx*output_nii.image->ny*output_nii.image->nz*output_nii.image->nt;
	output_nii.image->nbyper = 4;
	output_nii.image->datatype = NIFTI_TYPE_FLOAT32;
	output_nii.image->scl_slope = 0;
	output_nii.image->scl_inter = 0;
	output_nii.image->data = output.begin();
	nifti_image_write(output_nii.image);
	output_nii.image->data = nullptr;

	return EXIT_SUCCESS;
}
