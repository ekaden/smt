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

#ifndef _NIFTI_H
#define _NIFTI_H

#include <cstdlib>
#include <iostream>
#include <string>

#include "nifti1_io.h"

#include "debug.h"

namespace smt {

// TODO: Major revision

template <typename T, unsigned int D>
class nifti {
public:
	nifti_image* image;

	nifti(): image(nullptr) {}

	nifti(const std::string& filename, const bool load_data = false):
			image(nifti_image_read(filename.c_str(), (load_data)? 1 : 0)) {
		if(image == nullptr) {
			std::cerr << "ERROR: Unable to read '" << filename << "'." << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}

	void read(const std::string& filename, const bool load_data = false) {
		image = nifti_image_read(filename.c_str(), (load_data)? 1 : 0);
		if(image == nullptr) {
			std::cerr << "ERROR: Unable to read '" << filename << "'." << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}

	explicit operator bool() const {
		return image != nullptr;
	}

	T operator()(const std::size_t& i0) const {
		static_assert(D == 1, "D == 1");
		insist(0 <= i0 && i0 < size(0));

		T ret;
		switch(image->datatype) {
		case NIFTI_TYPE_INT8:
			ret = reinterpret_cast<signed char*>(image->data)[i0];
			break;
		case NIFTI_TYPE_UINT8:
			ret = reinterpret_cast<unsigned char*>(image->data)[i0];
			break;
		case NIFTI_TYPE_INT16:
			ret = reinterpret_cast<signed short int*>(image->data)[i0];
			break;
		case NIFTI_TYPE_UINT16:
			ret = reinterpret_cast<unsigned short int*>(image->data)[i0];
			break;
		case NIFTI_TYPE_INT32:
			ret = reinterpret_cast<signed int*>(image->data)[i0];
			break;
		case NIFTI_TYPE_UINT32:
			ret = reinterpret_cast<unsigned int*>(image->data)[i0];
			break;
		case NIFTI_TYPE_INT64:
			ret = reinterpret_cast<signed long int*>(image->data)[i0];
			break;
		case NIFTI_TYPE_UINT64:
			ret = reinterpret_cast<unsigned long int*>(image->data)[i0];
			break;
		case NIFTI_TYPE_FLOAT32:
			ret = reinterpret_cast<float*>(image->data)[i0];
			break;
		case NIFTI_TYPE_FLOAT64:
			ret = reinterpret_cast<double*>(image->data)[i0];
			break;
		default:
			std::cerr << "ERROR: Unable to read NIfTI-1 data type" << std::endl;
			std::exit(EXIT_FAILURE);
			break;
		}

		return ret;
	}

	T operator()(const std::size_t& i0, const std::size_t& i1) const {
		static_assert(D == 2, "D == 2");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1));

		T ret;
		switch(image->datatype) {
		case NIFTI_TYPE_INT8:
			ret = reinterpret_cast<signed char*>(image->data)[i0+size(0)*i1];
			break;
		case NIFTI_TYPE_UINT8:
			ret = reinterpret_cast<unsigned char*>(image->data)[i0+size(0)*i1];
			break;
		case NIFTI_TYPE_INT16:
			ret = reinterpret_cast<signed short int*>(image->data)[i0+size(0)*i1];
			break;
		case NIFTI_TYPE_UINT16:
			ret = reinterpret_cast<unsigned short int*>(image->data)[i0+size(0)*i1];
			break;
		case NIFTI_TYPE_INT32:
			ret = reinterpret_cast<signed int*>(image->data)[i0+size(0)*i1];
			break;
		case NIFTI_TYPE_UINT32:
			ret = reinterpret_cast<unsigned int*>(image->data)[i0+size(0)*i1];
			break;
		case NIFTI_TYPE_INT64:
			ret = reinterpret_cast<signed long int*>(image->data)[i0+size(0)*i1];
			break;
		case NIFTI_TYPE_UINT64:
			ret = reinterpret_cast<unsigned long int*>(image->data)[i0+size(0)*i1];
			break;
		case NIFTI_TYPE_FLOAT32:
			ret = reinterpret_cast<float*>(image->data)[i0+size(0)*i1];
			break;
		case NIFTI_TYPE_FLOAT64:
			ret = reinterpret_cast<double*>(image->data)[i0+size(0)*i1];
			break;
		default:
			std::cerr << "ERROR: Unable to read NIfTI-1 data type" << std::endl;
			std::exit(EXIT_FAILURE);
			break;
		}

		return ret;
	}

	T operator()(const std::size_t& i0, const std::size_t& i1, const std::size_t& i2) const {
		static_assert(D == 3, "D == 3");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1) && 0 <= i2 && i2 < size(2));

		T ret;
		switch(image->datatype) {
		case NIFTI_TYPE_INT8:
			ret = reinterpret_cast<signed char*>(image->data)[i0+size(0)*(i1+size(1)*i2)];
			break;
		case NIFTI_TYPE_UINT8:
			ret = reinterpret_cast<unsigned char*>(image->data)[i0+size(0)*(i1+size(1)*i2)];
			break;
		case NIFTI_TYPE_INT16:
			ret = reinterpret_cast<signed short int*>(image->data)[i0+size(0)*(i1+size(1)*i2)];
			break;
		case NIFTI_TYPE_UINT16:
			ret = reinterpret_cast<unsigned short int*>(image->data)[i0+size(0)*(i1+size(1)*i2)];
			break;
		case NIFTI_TYPE_INT32:
			ret = reinterpret_cast<signed int*>(image->data)[i0+size(0)*(i1+size(1)*i2)];
			break;
		case NIFTI_TYPE_UINT32:
			ret = reinterpret_cast<unsigned int*>(image->data)[i0+size(0)*(i1+size(1)*i2)];
			break;
		case NIFTI_TYPE_INT64:
			ret = reinterpret_cast<signed long int*>(image->data)[i0+size(0)*(i1+size(1)*i2)];
			break;
		case NIFTI_TYPE_UINT64:
			ret = reinterpret_cast<unsigned long int*>(image->data)[i0+size(0)*(i1+size(1)*i2)];
			break;
		case NIFTI_TYPE_FLOAT32:
			ret = reinterpret_cast<float*>(image->data)[i0+size(0)*(i1+size(1)*i2)];
			break;
		case NIFTI_TYPE_FLOAT64:
			ret = reinterpret_cast<double*>(image->data)[i0+size(0)*(i1+size(1)*i2)];
			break;
		default:
			std::cerr << "ERROR: Unable to read NIfTI-1 data type" << std::endl;
			std::exit(EXIT_FAILURE);
			break;
		}

		return ret;
	}

	T operator()(const std::size_t& i0, const std::size_t& i1, const std::size_t& i2, const std::size_t& i3) const {
		static_assert(D == 4, "D == 4");
		insist(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1) && 0 <= i2 && i2 < size(2) && 0 <= i3 && i3 < size(3));

		T ret;
		switch(image->datatype) {
		case NIFTI_TYPE_INT8:
			ret = reinterpret_cast<signed char*>(image->data)[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
			break;
		case NIFTI_TYPE_UINT8:
			ret = reinterpret_cast<unsigned char*>(image->data)[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
			break;
		case NIFTI_TYPE_INT16:
			ret = reinterpret_cast<signed short int*>(image->data)[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
			break;
		case NIFTI_TYPE_UINT16:
			ret = reinterpret_cast<unsigned short int*>(image->data)[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
			break;
		case NIFTI_TYPE_INT32:
			ret = reinterpret_cast<signed int*>(image->data)[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
			break;
		case NIFTI_TYPE_UINT32:
			ret = reinterpret_cast<unsigned int*>(image->data)[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
			break;
		case NIFTI_TYPE_INT64:
			ret = reinterpret_cast<signed long int*>(image->data)[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
			break;
		case NIFTI_TYPE_UINT64:
			ret = reinterpret_cast<unsigned long int*>(image->data)[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
			break;
		case NIFTI_TYPE_FLOAT32:
			ret = reinterpret_cast<float*>(image->data)[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
			break;
		case NIFTI_TYPE_FLOAT64:
			ret = reinterpret_cast<double*>(image->data)[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
			break;
		default:
			std::cerr << "ERROR: Unable to read NIfTI-1 data type" << std::endl;
			std::exit(EXIT_FAILURE);
			break;
		}

		return ret;
	}
	
	smt::darray<T, 1> operator()(const std::size_t& i0, const std::size_t& i1, const std::size_t& i2, const smt::slice& slice) const {
		static_assert(D == 4, "D == 4");
		smt::darray<T, 1> ret(slice.size());
		std::size_t i3 = slice.start();
		for(std::size_t ii = 0; ii < ret.size(); ++ii) {
			ret(ii) = operator()(i0, i1, i2, i3);
			i3 += slice.stride();
		}
		return ret;
	}

	std::size_t size() const {
		std::size_t total_size = 1;
		for(std::size_t ii = 0; ii < D; ++ii) {
			total_size *= image->dim[ii+1];
		}
		return total_size;
	}

	std::size_t size(const std::size_t& ii) const {
		insist(0 <= ii && ii < D);
		return image->dim[ii+1];
	}

	~nifti() {
		nifti_image_free(image);
	}
};

} // smt

#endif // _NIFTI_H
