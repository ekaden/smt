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
#include <string>

#include "docopt.h"

#include "darray.h"
#include "nifti.h"
#include "ricianfit.h"
#include "sarray.h"

static const char VERSION[] = R"(ricianfit v0.1)";

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
RICIAN NOISE ESTIMATION

Copyright (c) 2016 Enrico Kaden & University College London

Usage:
  ricianfit [options] <input> <output>
  ricianfit (-h | --help)
  ricianfit --license
  ricianfit --version

Options:
  --mask <mask>  Foreground mask [default: none]
  -h, --help     Help screen
  --license      License information
  --version      Software version
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
	if(input.size(3) < 2) {
		std::cerr << "ERROR: '" << args["<input>"].asString() << "' includes less than two volumes." << std::endl;
		return EXIT_FAILURE;
	}

	if(nifti_validfilename(args["<output>"].asString().c_str()) == 0) {
		std::cerr << "ERROR: '" << args["<output>"].asString().c_str() << "' is not a valid NIfTI-1 file name." << std::endl;
		return EXIT_FAILURE;
	}

	smt::nifti<float_t, 3> mask;
	if(args["--mask"] && args["--mask"].asString() != "none") {
		mask.read(args["--mask"].asString(), true);
		if(input.size(0) != mask.size(0) || input.size(1) != mask.size(1) || input.size(2) != mask.size(2)) {
			std::cerr << "ERROR: '" << args["<input>"].asString() << "' and '" << args["--mask"].asString() << "' do not match." << std::endl;
			return EXIT_FAILURE;
		}
	}

	// Processing

	smt::darray<float, 4> output(input.size(0), input.size(1), input.size(2), 2);
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
						input_tmp[ll] = input(ii, jj, kk, ll);
					}

					smt::sarray<float_t, 2> ricianfit_tmp = smt::ricianfit(input_tmp);
					output.colmaj(ii, jj, kk, 0) = ricianfit_tmp(0);
					output.colmaj(ii, jj, kk, 1) = ricianfit_tmp(1);
				} else {
					output.colmaj(ii, jj, kk, 0) = 0;
					output.colmaj(ii, jj, kk, 1) = 0;
				}
			}
		}
	}

	// Output

	// TODO: Separate NIfTI-1 output (e.g. <output>_signal.nii and <output>_noise.nii).
	smt::nifti<float, 4> output_nii;
	output_nii.image = nifti_copy_nim_info(input.image);
	nifti_set_filenames(output_nii.image, args["<output>"].asString().c_str(), 0, 0);
	output_nii.image->nt = 2;
	output_nii.image->dim[4] = 2;
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
