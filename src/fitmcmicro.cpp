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
#include "fitmcmicro.h"
#include "nifti.h"
#include "ricedebias.h"
#include "sarray.h"

static const char VERSION[] = R"(fitmcmicro v0.1)";

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
MULTI-COMPARTMENT MICROSCOPIC DIFFUSION IMAGING (SPHERICAL MEAN TECHNIQUE)

Copyright (c) 2016 Enrico Kaden & University College London

If you use this software, please cite:
  Kaden E, Kelm ND, Carson RP, Does MD, and Alexander DC: Multi-
  compartment microscopic diffusion imaging. NeuroImage, 139:346–359,
  2016.  http://dx.doi.org/10.1016/j.neuroimage.2016.06.002

Usage:
  fitmcmicro [options] <input> <output>
  fitmcmicro (-h | --help)
  fitmcmicro --license
  fitmcmicro --version

Options:
  --bvals <bvals>      Diffusion weighting factors (s/mm²)
  --bvecs <bvecs>      Diffusion gradient directions
  --graddev <graddev>  Diffusion gradient deviation [default: none]
  --mask <mask>        Foreground mask [default: none]
  --rician <rician>    Rician noise [default: none]
  --maxdiff <maxdiff>  Maximum diffusivity (mm²/s) [default: 3.05e-3]
  --b0                 Model-based estimation of zero b-value signal
  -h, --help           Help screen
  --license            License information
  --version            Software version
)";

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

	smt::diffenc_t<float_t> diffenc(args["--bvals"].asString(), args["--bvecs"].asString());
	if(input.size(3) != diffenc.mapping.size(0)) {
		std::cerr << "ERROR: '" << args["<input>"].asString() << "' and '" << args["--bvals"].asString() << "' and/or '" << args["--bvecs"].asString() << "' do not match." << std::endl;
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

					smt::diffenc_t<float_t> diffenc_tmp(diffenc);
					if(graddev) {
						smt::sarray<float_t, 3, 3> graddev_tmp;
						graddev_tmp(0, 0) = graddev(ii, jj, kk, 0);
						graddev_tmp(1, 0) = graddev(ii, jj, kk, 1);
						graddev_tmp(2, 0) = graddev(ii, jj, kk, 2);
						graddev_tmp(0, 1) = graddev(ii, jj, kk, 3);
						graddev_tmp(1, 1) = graddev(ii, jj, kk, 4);
						graddev_tmp(2, 1) = graddev(ii, jj, kk, 5);
						graddev_tmp(0, 2) = graddev(ii, jj, kk, 6);
						graddev_tmp(1, 2) = graddev(ii, jj, kk, 7);
						graddev_tmp(2, 2) = graddev(ii, jj, kk, 8);
						diffenc_tmp.corrdiffenc(graddev_tmp);
					}

					smt::sarray<float_t, 3> fitmcmicro_tmp = smt::fitmcmicro(input_tmp, diffenc_tmp, maxdiff, b0);
					output.colmaj(ii, jj, kk, 0) = fitmcmicro_tmp(0);
					output.colmaj(ii, jj, kk, 1) = fitmcmicro_tmp(1);
					output.colmaj(ii, jj, kk, 2) = (float_t(1)-fitmcmicro_tmp(0))*fitmcmicro_tmp(1);
					output.colmaj(ii, jj, kk, 3) = (float_t(1)-float_t(2)/float_t(3)*fitmcmicro_tmp(0))*fitmcmicro_tmp(1);
					output.colmaj(ii, jj, kk, 4) = fitmcmicro_tmp(2);
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

	// TODO: Separate NIfTI-1 output (e.g. <output>_intra.nii, <output>_diff.nii, <output>_extra_diff_trans, <output>_extra_diff_mean.nii, <output>_b0.ni).
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
