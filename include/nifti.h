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

#ifndef _NIFTI_H
#define _NIFTI_H

#include <algorithm>
#include <cctype>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <tuple>
#include <utility>

#include <sys/mman.h>
#ifdef ZLIB_FOUND
#include <zlib.h>
#endif // ZLIB_FOUND

#include "nifti1.h"

#include "darray.h"
#include "debug.h"
#include "sarray.h"

namespace smt {

namespace {

static_assert(sizeof(signed char) == 1, "sizeof(signed char) == 1");
static_assert(sizeof(unsigned char) == 1, "sizeof(unsigned char) == 1");
static_assert(sizeof(signed short int) == 2, "sizeof(signed short int) == 2");
static_assert(sizeof(unsigned short int) == 2, "sizeof(unsigned short int) == 2");
static_assert(sizeof(signed int) == 4, "sizeof(signed int) == 4");
static_assert(sizeof(unsigned int) == 4, "sizeof(unsigned int) == 4");
static_assert(sizeof(signed long int) == 8, "sizeof(signed long int) == 8");
static_assert(sizeof(unsigned long int) == 8, "sizeof(unsigned long int) == 8");
static_assert(sizeof(float) == 4, "sizeof(float) == 4");
static_assert(sizeof(double) == 8, "sizeof(double) == 8");
static_assert(sizeof(long double) == 16, "sizeof(float) == 16");
static_assert(sizeof(std::complex<float>) == 8, "sizeof(std::complex<float>) == 8");
static_assert(sizeof(std::complex<double>) == 16, "sizeof(std::complex<double>) == 16");
static_assert(sizeof(std::complex<long double>) == 32, "sizeof(std::complex<float>) == 32");

std::size_t nifti_bytesize(const short& datatype) {
	switch(datatype) {
	case NIFTI_TYPE_INT8:
		return sizeof(signed char);
	case NIFTI_TYPE_UINT8:
		return sizeof(unsigned char);
	case NIFTI_TYPE_INT16:
		return sizeof(signed short int);
	case NIFTI_TYPE_UINT16:
		return sizeof(unsigned short int);
	case NIFTI_TYPE_INT32:
		return sizeof(signed int);
	case NIFTI_TYPE_UINT32:
		return sizeof(unsigned int);
	case NIFTI_TYPE_INT64:
		return sizeof(signed long int);
	case NIFTI_TYPE_UINT64:
		return sizeof(unsigned long int);
	case NIFTI_TYPE_FLOAT32:
		return sizeof(float);
	case NIFTI_TYPE_FLOAT64:
		return sizeof(double);
	case NIFTI_TYPE_FLOAT128:
		return sizeof(long double);
	case NIFTI_TYPE_COMPLEX64:
		return sizeof(std::complex<float>);
	case NIFTI_TYPE_COMPLEX128:
		return sizeof(std::complex<double>);
	case NIFTI_TYPE_COMPLEX256:
		return sizeof(std::complex<long double>);
	default:
		smt::error("Unable to read/write NIfTI-1 data type.");
		std::exit(EXIT_FAILURE);
	}
	return 0; // unreachable
}

template <typename T>
short nifti_datatype() {
	smt::error("Unable to write NIfTI-1 data type.");
	std::exit(EXIT_FAILURE);
}

#ifndef DEFINE_NIFTI_DATATYPE
#define DEFINE_NIFTI_DATATYPE(INPUT_T, OUTPUT_D) \
		template <> \
		short nifti_datatype<INPUT_T>() { \
			return OUTPUT_D; \
		}

DEFINE_NIFTI_DATATYPE(signed char, NIFTI_TYPE_INT8)
DEFINE_NIFTI_DATATYPE(unsigned char, NIFTI_TYPE_UINT8)
DEFINE_NIFTI_DATATYPE(signed short int, NIFTI_TYPE_INT16)
DEFINE_NIFTI_DATATYPE(unsigned short int, NIFTI_TYPE_UINT16)
DEFINE_NIFTI_DATATYPE(signed int, NIFTI_TYPE_INT32)
DEFINE_NIFTI_DATATYPE(unsigned int, NIFTI_TYPE_UINT32)
DEFINE_NIFTI_DATATYPE(signed long int, NIFTI_TYPE_INT64)
DEFINE_NIFTI_DATATYPE(unsigned long int, NIFTI_TYPE_UINT64)
DEFINE_NIFTI_DATATYPE(float, NIFTI_TYPE_FLOAT32)
DEFINE_NIFTI_DATATYPE(double, NIFTI_TYPE_FLOAT64)
DEFINE_NIFTI_DATATYPE(long double, NIFTI_TYPE_FLOAT128)
DEFINE_NIFTI_DATATYPE(std::complex<float>, NIFTI_TYPE_COMPLEX64)
DEFINE_NIFTI_DATATYPE(std::complex<double>, NIFTI_TYPE_COMPLEX128)
DEFINE_NIFTI_DATATYPE(std::complex<long double>, NIFTI_TYPE_COMPLEX256)

#undef DEFINE_NIFTI_DATATYPE
#endif // DEFINE_NIFTI_DATATYPE

template <typename input_t, typename output_t, bool scaling>
output_t nifti_readfun(const std::size_t& ii, const unsigned char* data, const float& slope = 1.0f, const float& offset = 0.0f) {
	smt::error("Unable to read NIfTI-1 data type.");
	std::exit(EXIT_FAILURE);
}

#ifndef DEFINE_NIFTI_READFUN_UNSCALED
#define DEFINE_NIFTI_READFUN_UNSCALED(INPUT_T, OUTPUT_T) \
		template <> \
		OUTPUT_T nifti_readfun<INPUT_T, OUTPUT_T, false>(const std::size_t& ii, const unsigned char* data, const float& slope, const float& offset) { \
			return reinterpret_cast<const INPUT_T*>(data)[ii]; \
		}

// int8_t
DEFINE_NIFTI_READFUN_UNSCALED(signed char, signed char)
DEFINE_NIFTI_READFUN_UNSCALED(signed char, signed short)
DEFINE_NIFTI_READFUN_UNSCALED(signed char, signed int)
DEFINE_NIFTI_READFUN_UNSCALED(signed char, signed long)
DEFINE_NIFTI_READFUN_UNSCALED(signed char, float)
DEFINE_NIFTI_READFUN_UNSCALED(signed char, double)
DEFINE_NIFTI_READFUN_UNSCALED(signed char, long double)
DEFINE_NIFTI_READFUN_UNSCALED(signed char, std::complex<float>)
DEFINE_NIFTI_READFUN_UNSCALED(signed char, std::complex<double>)
DEFINE_NIFTI_READFUN_UNSCALED(signed char, std::complex<long double>)

// int16_t
DEFINE_NIFTI_READFUN_UNSCALED(signed short, signed short)
DEFINE_NIFTI_READFUN_UNSCALED(signed short, signed int)
DEFINE_NIFTI_READFUN_UNSCALED(signed short, signed long)
DEFINE_NIFTI_READFUN_UNSCALED(signed short, float)
DEFINE_NIFTI_READFUN_UNSCALED(signed short, double)
DEFINE_NIFTI_READFUN_UNSCALED(signed short, long double)
DEFINE_NIFTI_READFUN_UNSCALED(signed short, std::complex<float>)
DEFINE_NIFTI_READFUN_UNSCALED(signed short, std::complex<double>)
DEFINE_NIFTI_READFUN_UNSCALED(signed short, std::complex<long double>)

// int32_t
DEFINE_NIFTI_READFUN_UNSCALED(signed int, signed int)
DEFINE_NIFTI_READFUN_UNSCALED(signed int, signed long)
DEFINE_NIFTI_READFUN_UNSCALED(signed int, float)
DEFINE_NIFTI_READFUN_UNSCALED(signed int, double)
DEFINE_NIFTI_READFUN_UNSCALED(signed int, long double)
DEFINE_NIFTI_READFUN_UNSCALED(signed int, std::complex<float>)
DEFINE_NIFTI_READFUN_UNSCALED(signed int, std::complex<double>)
DEFINE_NIFTI_READFUN_UNSCALED(signed int, std::complex<long double>)

// int64_t
DEFINE_NIFTI_READFUN_UNSCALED(signed long, signed long)
DEFINE_NIFTI_READFUN_UNSCALED(signed long, float)
DEFINE_NIFTI_READFUN_UNSCALED(signed long, double)
DEFINE_NIFTI_READFUN_UNSCALED(signed long, long double)
DEFINE_NIFTI_READFUN_UNSCALED(signed long, std::complex<float>)
DEFINE_NIFTI_READFUN_UNSCALED(signed long, std::complex<double>)
DEFINE_NIFTI_READFUN_UNSCALED(signed long, std::complex<long double>)

// uint8_t
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, unsigned char)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, signed short)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, unsigned short)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, signed int)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, unsigned int)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, signed long)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, unsigned long)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, float)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, double)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, long double)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, std::complex<float>)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, std::complex<double>)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned char, std::complex<long double>)

// uint16_t
DEFINE_NIFTI_READFUN_UNSCALED(unsigned short, unsigned short)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned short, signed int)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned short, unsigned int)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned short, signed long)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned short, unsigned long)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned short, float)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned short, double)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned short, long double)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned short, std::complex<float>)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned short, std::complex<double>)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned short, std::complex<long double>)

// uint32_t
DEFINE_NIFTI_READFUN_UNSCALED(unsigned int, unsigned int)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned int, signed long)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned int, unsigned long)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned int, float)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned int, double)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned int, long double)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned int, std::complex<float>)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned int, std::complex<double>)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned int, std::complex<long double>)

// uint64_t
DEFINE_NIFTI_READFUN_UNSCALED(unsigned long, unsigned long)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned long, float)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned long, double)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned long, long double)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned long, std::complex<float>)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned long, std::complex<double>)
DEFINE_NIFTI_READFUN_UNSCALED(unsigned long, std::complex<long double>)

// float32_t
DEFINE_NIFTI_READFUN_UNSCALED(float, float)
DEFINE_NIFTI_READFUN_UNSCALED(float, double)
DEFINE_NIFTI_READFUN_UNSCALED(float, long double)
DEFINE_NIFTI_READFUN_UNSCALED(float, std::complex<float>)
DEFINE_NIFTI_READFUN_UNSCALED(float, std::complex<double>)
DEFINE_NIFTI_READFUN_UNSCALED(float, std::complex<long double>)

// float64_t
DEFINE_NIFTI_READFUN_UNSCALED(double, double)
DEFINE_NIFTI_READFUN_UNSCALED(double, long double)
DEFINE_NIFTI_READFUN_UNSCALED(double, std::complex<double>)
DEFINE_NIFTI_READFUN_UNSCALED(double, std::complex<long double>)

// float128_t
DEFINE_NIFTI_READFUN_UNSCALED(long double, long double)
DEFINE_NIFTI_READFUN_UNSCALED(long double, std::complex<long double>)

#undef DEFINE_NIFTI_READFUN_UNSCALED
#endif // DEFINE_NIFTI_READFUN_UNSCALED

#ifndef DEFINE_NIFTI_READFUN_COMPLEX_UNSCALED
#define DEFINE_NIFTI_READFUN_COMPLEX_UNSCALED(INPUT_T, OUTPUT_T) \
		template <> \
		OUTPUT_T nifti_readfun<INPUT_T, OUTPUT_T, false>(const std::size_t& ii, const unsigned char* data, const float& slope, const float& offset) { \
			return reinterpret_cast<const INPUT_T*>(data)[ii]; \
		}

// complex64_t
DEFINE_NIFTI_READFUN_COMPLEX_UNSCALED(std::complex<float>, std::complex<float>)
DEFINE_NIFTI_READFUN_COMPLEX_UNSCALED(std::complex<float>, std::complex<double>)
DEFINE_NIFTI_READFUN_COMPLEX_UNSCALED(std::complex<float>, std::complex<long double>)

// complex128_t
DEFINE_NIFTI_READFUN_COMPLEX_UNSCALED(std::complex<double>, std::complex<double>)
DEFINE_NIFTI_READFUN_COMPLEX_UNSCALED(std::complex<double>, std::complex<long double>)

// complex256_t
DEFINE_NIFTI_READFUN_COMPLEX_UNSCALED(std::complex<long double>, std::complex<long double>)

#undef DEFINE_NIFTI_READFUN_COMPLEX_UNSCALED
#endif // DEFINE_NIFTI_READFUN_COMPLEX_UNSCALED

#ifndef DEFINE_NIFTI_READFUN_SCALED
#define DEFINE_NIFTI_READFUN_SCALED(INPUT_T, OUTPUT_T) \
		template <> \
		OUTPUT_T nifti_readfun<INPUT_T, OUTPUT_T, true>(const std::size_t& ii, const unsigned char* data, const float& slope, const float& offset) { \
			return slope*reinterpret_cast<const INPUT_T*>(data)[ii]+offset; \
		}

// int8_t
DEFINE_NIFTI_READFUN_SCALED(signed char, float)
DEFINE_NIFTI_READFUN_SCALED(signed char, double)
DEFINE_NIFTI_READFUN_SCALED(signed char, long double)
DEFINE_NIFTI_READFUN_SCALED(signed char, std::complex<float>)
DEFINE_NIFTI_READFUN_SCALED(signed char, std::complex<double>)
DEFINE_NIFTI_READFUN_SCALED(signed char, std::complex<long double>)

// int16_t
DEFINE_NIFTI_READFUN_SCALED(signed short, float)
DEFINE_NIFTI_READFUN_SCALED(signed short, double)
DEFINE_NIFTI_READFUN_SCALED(signed short, long double)
DEFINE_NIFTI_READFUN_SCALED(signed short, std::complex<float>)
DEFINE_NIFTI_READFUN_SCALED(signed short, std::complex<double>)
DEFINE_NIFTI_READFUN_SCALED(signed short, std::complex<long double>)

// int32_t
DEFINE_NIFTI_READFUN_SCALED(signed int, float)
DEFINE_NIFTI_READFUN_SCALED(signed int, double)
DEFINE_NIFTI_READFUN_SCALED(signed int, long double)
DEFINE_NIFTI_READFUN_SCALED(signed int, std::complex<float>)
DEFINE_NIFTI_READFUN_SCALED(signed int, std::complex<double>)
DEFINE_NIFTI_READFUN_SCALED(signed int, std::complex<long double>)

// int64_t
DEFINE_NIFTI_READFUN_SCALED(signed long, float)
DEFINE_NIFTI_READFUN_SCALED(signed long, double)
DEFINE_NIFTI_READFUN_SCALED(signed long, long double)
DEFINE_NIFTI_READFUN_SCALED(signed long, std::complex<float>)
DEFINE_NIFTI_READFUN_SCALED(signed long, std::complex<double>)
DEFINE_NIFTI_READFUN_SCALED(signed long, std::complex<long double>)

// uint8_t
DEFINE_NIFTI_READFUN_SCALED(unsigned char, float)
DEFINE_NIFTI_READFUN_SCALED(unsigned char, double)
DEFINE_NIFTI_READFUN_SCALED(unsigned char, long double)
DEFINE_NIFTI_READFUN_SCALED(unsigned char, std::complex<float>)
DEFINE_NIFTI_READFUN_SCALED(unsigned char, std::complex<double>)
DEFINE_NIFTI_READFUN_SCALED(unsigned char, std::complex<long double>)

// uint16_t
DEFINE_NIFTI_READFUN_SCALED(unsigned short, float)
DEFINE_NIFTI_READFUN_SCALED(unsigned short, double)
DEFINE_NIFTI_READFUN_SCALED(unsigned short, long double)
DEFINE_NIFTI_READFUN_SCALED(unsigned short, std::complex<float>)
DEFINE_NIFTI_READFUN_SCALED(unsigned short, std::complex<double>)
DEFINE_NIFTI_READFUN_SCALED(unsigned short, std::complex<long double>)

// uint32_t
DEFINE_NIFTI_READFUN_SCALED(unsigned int, float)
DEFINE_NIFTI_READFUN_SCALED(unsigned int, double)
DEFINE_NIFTI_READFUN_SCALED(unsigned int, long double)
DEFINE_NIFTI_READFUN_SCALED(unsigned int, std::complex<float>)
DEFINE_NIFTI_READFUN_SCALED(unsigned int, std::complex<double>)
DEFINE_NIFTI_READFUN_SCALED(unsigned int, std::complex<long double>)

// uint64_t
DEFINE_NIFTI_READFUN_SCALED(unsigned long, float)
DEFINE_NIFTI_READFUN_SCALED(unsigned long, double)
DEFINE_NIFTI_READFUN_SCALED(unsigned long, long double)
DEFINE_NIFTI_READFUN_SCALED(unsigned long, std::complex<float>)
DEFINE_NIFTI_READFUN_SCALED(unsigned long, std::complex<double>)
DEFINE_NIFTI_READFUN_SCALED(unsigned long, std::complex<long double>)

// float32_t
DEFINE_NIFTI_READFUN_SCALED(float, float)
DEFINE_NIFTI_READFUN_SCALED(float, double)
DEFINE_NIFTI_READFUN_SCALED(float, long double)
DEFINE_NIFTI_READFUN_SCALED(float, std::complex<float>)
DEFINE_NIFTI_READFUN_SCALED(float, std::complex<double>)
DEFINE_NIFTI_READFUN_SCALED(float, std::complex<long double>)

// float64_t
DEFINE_NIFTI_READFUN_SCALED(double, double)
DEFINE_NIFTI_READFUN_SCALED(double, long double)
DEFINE_NIFTI_READFUN_SCALED(double, std::complex<double>)
DEFINE_NIFTI_READFUN_SCALED(double, std::complex<long double>)

// float128_t
DEFINE_NIFTI_READFUN_SCALED(long double, long double)
DEFINE_NIFTI_READFUN_SCALED(long double, std::complex<long double>)

#undef DEFINE_NIFTI_READFUN_SCALED
#endif // DEFINE_NIFTI_READFUN_SCALED

#ifndef DEFINE_NIFTI_READFUN_COMPLEX_SCALED
#define DEFINE_NIFTI_READFUN_COMPLEX_SCALED(INPUT_T, OUTPUT_T) \
		template <> \
		OUTPUT_T nifti_readfun<INPUT_T, OUTPUT_T, true>(const std::size_t& ii, const unsigned char* data, const float& slope, const float& offset) { \
			const INPUT_T::value_type slope_ = slope; \
			const INPUT_T::value_type offset_ = offset; \
			return slope_*reinterpret_cast<const INPUT_T*>(data)[ii]+offset_; \
		}

// complex64_t
DEFINE_NIFTI_READFUN_COMPLEX_SCALED(std::complex<float>, std::complex<float>)
DEFINE_NIFTI_READFUN_COMPLEX_SCALED(std::complex<float>, std::complex<double>)
DEFINE_NIFTI_READFUN_COMPLEX_SCALED(std::complex<float>, std::complex<long double>)

// complex128_t
DEFINE_NIFTI_READFUN_COMPLEX_SCALED(std::complex<double>, std::complex<double>)
DEFINE_NIFTI_READFUN_COMPLEX_SCALED(std::complex<double>, std::complex<long double>)

// complex256_t
DEFINE_NIFTI_READFUN_COMPLEX_SCALED(std::complex<long double>, std::complex<long double>)

#undef DEFINE_NIFTI_READFUN_COMPLEX_SCALED
#endif // DEFINE_NIFTI_READFUN_COMPLEX_SCALED

int fileno(std::FILE* f) {
	if(f == nullptr) {
		return -1;
	} else {
		return ::fileno(f);
	}
}

#ifdef ZLIB_FOUND
std::ptrdiff_t gzskip(gzFile f, std::ptrdiff_t n) {
	smt::darray<unsigned char, 1> buf(n);
	return gzread(f, buf.begin(), n);
}
#endif // ZLIB_FOUND

std::ptrdiff_t fskip(std::FILE* f, std::ptrdiff_t n) {
	smt::darray<unsigned char, 1> buf(n);
	return std::fread(buf.begin(), 1, n, f);
}

bool has_nifti_extension(const std::string& filename, const std::string& ext, const bool& icase = true) {
	if(filename.length() >= ext.length()) {
		const std::string::size_type offset = filename.length()-ext.length();
		if(icase) {
			for(std::string::size_type ii = 0; ii < ext.length(); ++ii) {
				if(::isupper(filename[offset+ii]) != 0) {
					if(filename[offset+ii] != ::toupper(ext[ii])) {
						return false;
					}
				} else {
					if(filename[offset+ii] != ::tolower(ext[ii])) {
						return false;
					}
				}
			}
		} else {
			for(std::string::size_type ii = 0; ii < ext.length(); ++ii) {
				if(filename[offset+ii] != ext[ii]) {
					return false;
				}
			}
		}
		return true;
	} else {
		return false;
	}
}

std::string replace_nifti_extension(const std::string& filename, const std::string& ext, const bool& icase = true) {
	if(filename.length() >= ext.length()) {
		const std::string::size_type offset = filename.length()-ext.length();
		std::string ret = filename;
		if(icase) {
			for(std::string::size_type ii = 0; ii < ext.length(); ++ii) {
				if(::isupper(filename[offset+ii]) != 0) {
					ret[offset+ii] = ::toupper(ext[ii]);
				} else {
					ret[offset+ii] = ::tolower(ext[ii]);
				}
			}
		} else {
			for(std::string::size_type ii = 0; ii < ext.length(); ++ii) {
				ret[offset+ii] = ext[ii];
			}
		}
		return ret;
	} else {
		return std::string();
	}
}

std::tuple<bool, bool, std::string, std::string> niftiname(const std::string& filename) {
	if(has_nifti_extension(filename, ".gz")) {
#ifdef ZLIB_FOUND
		if(has_nifti_extension(filename, ".hdr.gz") || has_nifti_extension(filename, ".img.gz")) {
			return std::make_tuple(true, true, replace_nifti_extension(filename, ".hdr.gz"), replace_nifti_extension(filename, ".img.gz"));
		} else {
			return std::make_tuple(true, false, filename, filename);
		}
#else
		smt::error("Built without support for gzip format.");
		std::exit(EXIT_FAILURE);
#endif // ZLIB_FOUND
	} else {
		if(has_nifti_extension(filename, ".hdr") || has_nifti_extension(filename, ".img")) {
			return std::make_tuple(false, true, replace_nifti_extension(filename, ".hdr"), replace_nifti_extension(filename, ".img"));
		} else {
			return std::make_tuple(false, false, filename, filename);
		}
	}
}

template <typename float_t>
bool approximately_equal(const float_t& a, const float_t& b);

template <>
bool approximately_equal(const float& a, const float& b) {
	return std::abs(a-b) <= 100*std::numeric_limits<float>::epsilon()*std::max(std::abs(a), std::abs(b));
}

template <>
bool approximately_equal(const double& a, const double& b) {
	return std::abs(a-b) <= 100*std::numeric_limits<double>::epsilon()*std::max(std::abs(a), std::abs(b));
}

template <>
bool approximately_equal(const long double& a, const long double& b) {
	return std::abs(a-b) <= 100*std::numeric_limits<long double>::epsilon()*std::max(std::abs(a), std::abs(b));
}

} // (anonymous)

template <typename T, unsigned int D>
class inifti {
	template <typename Tlike, unsigned int Dlike>
	friend class inifti;
	template <typename Tlike, unsigned int Dlike>
	friend class onifti;

public:
	inifti():
		_gzipped(false),
		_separate_storage(false),
		_hdrname(),
		_imgname(),
#ifdef ZLIB_FOUND
		_fd(-1),
		_zin(nullptr),
#else
		_fin(nullptr),
#endif // ZLIB_FOUND
		_header(),
		_data(nullptr),
		_mmapped(false),
		_readfun() {
	}

	inifti(const std::string& filename): inifti(smt::niftiname(filename)) {
	}

	inifti(const inifti&) = delete;

	inifti(inifti&& rhs):
		_gzipped(std::move(rhs._gzipped)),
		_separate_storage(std::move(rhs._separate_storage)),
		_hdrname(std::move(rhs._hdrname)),
		_imgname(std::move(rhs._imgname)) {
#ifdef ZLIB_FOUND
		_fd = std::move(rhs._fd);
		_zin = std::move(rhs._zin);
#else
		_fin = std::move(rhs._fin);
#endif
		_header = std::move(rhs._header);
		_data = std::move(rhs._data);
		rhs._data = nullptr;
		_mmapped = std::move(rhs._mmapped);
		_readfun = std::move(rhs._readfun);
	}

	inifti& operator=(const inifti&) = delete;

	inifti& operator=(inifti&&) = delete;

	explicit operator bool() const {
		return _data != nullptr;
	}

	T operator[](const std::size_t& ii) const {
		smt::assert(0 <= ii && ii < size());
		return _readfun(ii, _data, _header.scl_slope, _header.scl_inter);
	}

	T operator()(const std::size_t& i0, const std::size_t& i1, const std::size_t& i2) const {
		static_assert(D == 3, "D == 3");
		smt::assert(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1) && 0 <= i2 && i2 < size(2));
		return operator[](i0+size(0)*(i1+size(1)*i2));
	}

	T operator()(const std::size_t& i0, const std::size_t& i1, const std::size_t& i2, const std::size_t& i3) const {
		static_assert(D == 4, "D == 4");
		smt::assert(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1) && 0 <= i2 && i2 < size(2) && 0 <= i3 && i3 < size(3));
		return operator[](i0+size(0)*(i1+size(1)*(i2+size(2)*i3)));
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
			total_size *= _header.dim[ii+1];
		}
		return total_size;
	}

	std::size_t size(const std::size_t& ii) const {
		smt::assert(0 <= ii && ii < D);
		return _header.dim[ii+1];
	}

	float pixsize(const std::size_t& ii) const {
		smt::assert(0 <= ii && ii < D);
		return _header.pixdim[ii+1];
	}

	template <typename Tlike, unsigned int Dlike>
	bool has_equal_spatial_coords(const inifti<Tlike, Dlike>& like) const {
		if(_header.pixdim[0] == like._header.pixdim[0]
				&& _header.qform_code == like._header.qform_code
				&& _header.sform_code == like._header.sform_code
				&& smt::approximately_equal(_header.quatern_b, like._header.quatern_b)
				&& smt::approximately_equal(_header.quatern_c, like._header.quatern_c)
				&& smt::approximately_equal(_header.quatern_d, like._header.quatern_d)
				&& smt::approximately_equal(_header.qoffset_x, like._header.qoffset_x)
				&& smt::approximately_equal(_header.qoffset_y, like._header.qoffset_y)
				&& smt::approximately_equal(_header.qoffset_z, like._header.qoffset_z)
				&& std::equal(std::begin(_header.srow_x), std::end(_header.srow_x), std::begin(like._header.srow_x), smt::approximately_equal<float>)
				&& std::equal(std::begin(_header.srow_y), std::end(_header.srow_y), std::begin(like._header.srow_y), smt::approximately_equal<float>)
				&& std::equal(std::begin(_header.srow_z), std::end(_header.srow_z), std::begin(like._header.srow_z), smt::approximately_equal<float>)) {
			return true;
		} else {
			return false;
		}
	}

	~inifti() {
		if(operator bool()) {
			if(_mmapped) {
				if(_separate_storage) {
					if(munmap(_data-std::max(0L, offset()), bytesize()*size()+std::max(0L, offset())) != 0) {
						smt::error("Unable to munmap ‘" + _imgname + "’.");
						std::exit(EXIT_FAILURE);
					}
				} else {
					if(munmap(_data-std::max(352L, offset()), bytesize()*size()+std::max(352L, offset())) != 0) {
						smt::error("Unable to munmap ‘" + _imgname + "’.");
						std::exit(EXIT_FAILURE);
					}
				}
			} else {
				delete [] _data;
			}
#ifdef ZLIB_FOUND
			if(_fd != smt::fileno(stdin) && gzclose(_zin) != 0) {
				smt::error("Unable to close ‘" + _imgname + "’.");
				std::exit(EXIT_FAILURE);
			}
#else
			if(_fin != ::stdin && std::fclose(_fin) != 0) {
				smt::error("Unable to close ‘" + _imgname + "’.");
				std::exit(EXIT_FAILURE);
			}
#endif // ZLIB_FOUND
		}
	}

private:
	const bool _gzipped;
	const bool _separate_storage;
	const std::string _hdrname;
	const std::string _imgname;
#ifdef ZLIB_FOUND
	int _fd;
	gzFile _zin;
#else
	std::FILE* _fin;
#endif
	nifti_1_header _header;
	unsigned char* _data;
	bool _mmapped;
	std::function<T(const std::size_t&, const unsigned char*, const float&, const float&)> _readfun;

	inifti(const std::tuple<bool, bool, std::string, std::string>& niftiname):
			_gzipped(std::get<0>(niftiname)),
			_separate_storage(std::get<1>(niftiname)),
			_hdrname(std::get<2>(niftiname)),
			_imgname(std::get<3>(niftiname)) {
#ifdef ZLIB_FOUND
		if((_fd = (_hdrname == "-")? smt::fileno(stdin) : smt::fileno(std::fopen(_hdrname.c_str(), "rb"))) < 0 || (_zin = gzdopen(_fd, "rb")) == nullptr) {
			smt::error("Unable to open ‘" + _hdrname + "’.");
			std::exit(EXIT_FAILURE);
		}
		if(gzread(_zin, &_header, sizeof(nifti_1_header)) != sizeof(nifti_1_header)) {
			smt::error("Unable to read ‘" + _hdrname + "’.");
			std::exit(EXIT_FAILURE);
		}
#else
		if((_fin = (_hdrname == "-")? ::stdin : std::fopen(_hdrname.c_str(), "rb")) == nullptr) {
			smt::error("Unable to open ‘" + _hdrname + "’.");
			std::exit(EXIT_FAILURE);
		}
		if(fread(&_header, sizeof(nifti_1_header), 1, _fin) != 1) {
			smt::error("Unable to read ‘" + _hdrname + "’.");
			std::exit(EXIT_FAILURE);
		}
#endif // ZLIB_FOUND

		if(_separate_storage) {
			if(! has_magic_flag("ni1")) {
				smt::error("‘" + _hdrname + "’ not in NIfTI-1 format.");
				std::exit(EXIT_FAILURE);
			}
		} else {
			if(! has_magic_flag("n+1")) {
				smt::error("‘" + _hdrname + "’ not in NIfTI-1 format.");
				std::exit(EXIT_FAILURE);
			}
		}

		if(change_endianness()) {
			smt::error("Change of endianness in ‘" + _hdrname + "’ not supported.");
			std::exit(EXIT_FAILURE);
		}

		if(ndims() != D) {
			smt::error("Number of dimensions in ‘" + _hdrname + "’ not supported.");
			std::exit(EXIT_FAILURE);
		}

		if(! has_valid_size()) {
			smt::error("Dimensions in ‘" + _hdrname + "’ not non-negative.");
			std::exit(EXIT_FAILURE);
		}

		if(_gzipped) {
			if(_separate_storage) {
#ifdef ZLIB_FOUND
				if(gzclose(_zin) != 0) {
					smt::error("Unable to close ‘" + _hdrname + "’.");
					std::exit(EXIT_FAILURE);
				}
				if((_fd = smt::fileno(std::fopen(_imgname.c_str(), "rb"))) < 0 || (_zin = gzdopen(_fd, "rb")) == nullptr) {
					smt::error("Unable to open ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}
				if(gzskip(_zin, std::max(0L, offset())) != std::max(0L, offset())) {
					smt::error("Unable to read ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}

				if((_data = new unsigned char[bytesize()*size()]) == nullptr) {
					smt::error("Unable to allocate memory.");
					std::exit(EXIT_FAILURE);
				}
				if(gzread(_zin, _data, bytesize()*size()) != bytesize()*size()) {
					smt::error("Unable to read ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}
				_mmapped = false;
#else
				smt::error("Built without support for gzip format.");
				std::exit(EXIT_FAILURE);
#endif // ZLIB_FOUND
			} else {
#ifdef ZLIB_FOUND
				if(gzskip(_zin, std::max(352L, offset())-352L+4L) != std::max(352L, offset())-352L+4L) {
					smt::error("Unable to read ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}

				if((_data = new unsigned char[bytesize()*size()]) == nullptr) {
					smt::error("Unable to allocate memory.");
					std::exit(EXIT_FAILURE);
				}
				if(gzread(_zin, _data, bytesize()*size()) != bytesize()*size()) {
					smt::error("Unable to read ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}
				_mmapped = false;
#else
				smt::error("Built without support for gzip format.");
				std::exit(EXIT_FAILURE);
#endif // ZLIB_FOUND
			}
		} else {
			if(_separate_storage) {
#ifdef ZLIB_FOUND
				if(gzclose(_zin) != 0) {
					smt::error("Unable to close ‘" + _hdrname + "’.");
					std::exit(EXIT_FAILURE);
				}
				if((_fd = smt::fileno(std::fopen(_imgname.c_str(), "rb"))) < 0 || (_zin = gzdopen(_fd, "rb")) == nullptr) {
					smt::error("Unable to open ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}
				if(gzskip(_zin, std::max(0L, offset())) != std::max(0L, offset())) {
					smt::error("Unable to read ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}

				if((_mmapped = ((_data = static_cast<unsigned char*>(mmap(0, bytesize()*size()+std::max(0L, offset()), PROT_READ, MAP_SHARED, _fd, 0))) != MAP_FAILED))) {
					_data += std::max(0L, offset());
				} else {
					if((_data = new unsigned char[bytesize()*size()]) == nullptr) {
						smt::error("Unable to allocate memory.");
						std::exit(EXIT_FAILURE);
					}
					if(gzread(_zin, _data, bytesize()*size()) != bytesize()*size()) {
						smt::error("Unable to read ‘" + _imgname + "’.");
						std::exit(EXIT_FAILURE);
					}
				}
#else
				if(std::fclose(_fin) != 0) {
					smt::error("Unable to close ‘" + _hdrname + "’.");
					std::exit(EXIT_FAILURE);
				}
				if((_fin = fopen(_imgname.c_str(), "rb")) == nullptr) {
					smt::error("Unable to open ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}
				if(fskip(_fin, std::max(0L, offset())) != std::max(0L, offset())) {
					smt::error("Unable to read ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}

				if((_mmapped = ((_data = static_cast<unsigned char*>(mmap(0, bytesize()*size()+std::max(0L, offset()), PROT_READ, MAP_SHARED, smt::fileno(_fin), 0))) != MAP_FAILED))) {
					_data += std::max(0L, offset());
				} else {
					if((_data = new unsigned char[bytesize()*size()]) == nullptr) {
						smt::error("Unable to allocate memory.");
						std::exit(EXIT_FAILURE);
					}
					if(std::fread(_data, bytesize(), size(), _fin) != size()) {
						smt::error("Unable to read ‘" + _imgname + "’.");
						std::exit(EXIT_FAILURE);
					}
				}
#endif // ZLIB_FOUND
			} else {
#ifdef ZLIB_FOUND
				if(gzskip(_zin, std::max(352L, offset())-352L+4L) != std::max(352L, offset())-352L+4L) {
					smt::error("Unable to read ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}

				if((_mmapped = ((_data = static_cast<unsigned char*>(mmap(0, bytesize()*size()+std::max(352L, offset()), PROT_READ, MAP_SHARED, _fd, 0))) != MAP_FAILED))) {
					if(std::memcmp(&_header, _data, 348L) != 0) {
						if(munmap(_data, bytesize()*size()+std::max(352L, offset())) != 0) {
							smt::error("Unable to munmap ‘" + _imgname + "’.");
							std::exit(EXIT_FAILURE);
						}
						_mmapped = false;

						if((_data = new unsigned char[bytesize()*size()]) == nullptr) {
							smt::error("Unable to allocate memory.");
							std::exit(EXIT_FAILURE);
						}
						if(gzread(_zin, _data, bytesize()*size()) != bytesize()*size()) {
							smt::error("Unable to read ‘" + _imgname + "’.");
							std::exit(EXIT_FAILURE);
						}
					} else {
						_data += std::max(352L, offset());
					}
				} else {
					if((_data = new unsigned char[bytesize()*size()]) == nullptr) {
						smt::error("Unable to allocate memory.");
						std::exit(EXIT_FAILURE);
					}
					if(gzread(_zin, _data, bytesize()*size()) != bytesize()*size()) {
						smt::error("Unable to read ‘" + _imgname + "’.");
						std::exit(EXIT_FAILURE);
					}
				}
#else
				if(fskip(_fin, std::max(352L, offset())-352L+4L) != std::max(352L, offset())-352L+4L) {
					smt::error("Unable to read ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}

				if((_mmapped = ((_data = static_cast<unsigned char*>(mmap(0, bytesize()*size()+std::max(352L, offset()), PROT_READ, MAP_SHARED, smt::fileno(_fin), 0))) != MAP_FAILED))) {
					_data += std::max(352L, offset());
				} else {
					if((_data = new unsigned char[bytesize()*size()]) == nullptr) {
						smt::error("Unable to allocate memory.");
						std::exit(EXIT_FAILURE);
					}
					if(std::fread(_data, bytesize(), size(), _fin) != size()) {
						smt::error("Unable to read ‘" + _imgname + "’.");
						std::exit(EXIT_FAILURE);
					}
				}
#endif // ZLIB_FOUND
			}
		}

#ifndef DEFINE_NIFTI_READFUN
#define DEFINE_NIFTI_READFUN(OUTPUT_T) \
		if(_header.scl_slope == 0.0f || (_header.scl_slope == 1.0f && _header.scl_inter == 0.0f)) { \
			_readfun = &nifti_readfun<OUTPUT_T, T, false>; \
		} else { \
			_readfun = &nifti_readfun<OUTPUT_T, T, true>; \
		}

		switch(_header.datatype) {
		case NIFTI_TYPE_INT8:
			DEFINE_NIFTI_READFUN(signed char)
			break;
		case NIFTI_TYPE_UINT8:
			DEFINE_NIFTI_READFUN(unsigned char)
			break;
		case NIFTI_TYPE_INT16:
			DEFINE_NIFTI_READFUN(signed short int)
			break;
		case NIFTI_TYPE_UINT16:
			DEFINE_NIFTI_READFUN(unsigned short int)
			break;
		case NIFTI_TYPE_INT32:
			DEFINE_NIFTI_READFUN(signed int)
			break;
		case NIFTI_TYPE_UINT32:
			DEFINE_NIFTI_READFUN(unsigned int)
			break;
		case NIFTI_TYPE_INT64:
			DEFINE_NIFTI_READFUN(signed long int)
			break;
		case NIFTI_TYPE_UINT64:
			DEFINE_NIFTI_READFUN(unsigned long int)
			break;
		case NIFTI_TYPE_FLOAT32:
			DEFINE_NIFTI_READFUN(float)
			break;
		case NIFTI_TYPE_FLOAT64:
			DEFINE_NIFTI_READFUN(double)
			break;
		case NIFTI_TYPE_FLOAT128:
			DEFINE_NIFTI_READFUN(long double)
			break;
		case NIFTI_TYPE_COMPLEX64:
			DEFINE_NIFTI_READFUN(std::complex<float>)
			break;
		case NIFTI_TYPE_COMPLEX128:
			DEFINE_NIFTI_READFUN(std::complex<double>)
			break;
		case NIFTI_TYPE_COMPLEX256:
			DEFINE_NIFTI_READFUN(std::complex<long double>)
			break;
		default:
			smt::error("Unable to read NIfTI-1 data type.");
			std::exit(EXIT_FAILURE);
			break;
		}

#undef DEFINE_NIFTI_READFUN
#endif // DEFINE_NIFTI_READFUN
	}

	std::size_t bytesize() const {
		return nifti_bytesize(_header.datatype);
	}

	bool change_endianness() const {
		return ! (0 <= _header.dim[0] && _header.dim[0] <= 7);
	}

	bool has_magic_flag(const std::string& flag) const {
		return _header.magic == flag;
	}

	bool has_valid_size() const {
		return std::all_of(std::begin(_header.dim)+1u, std::begin(_header.dim)+1u+D, [&](const short& dim) {
			return dim >= 0;
		});
	}

	std::ptrdiff_t ndims() const {
		return _header.dim[0];
	}

	std::ptrdiff_t offset() const {
		return _header.vox_offset;
	}
};

template <typename T, unsigned int D>
class onifti {
	template <typename Tlike, unsigned int Dlike>
	friend class inifti;
	template <typename Tlike, unsigned int Dlike>
	friend class onifti;

public:
	onifti():
		_gzipped(false),
		_separate_storage(false),
		_hdrname(),
		_imgname(),
		_header(),
		_extender(),
		_fout(nullptr),
		_data(),
		_mmapped(false) {
	}

	template <typename Tlike, unsigned int Dlike>
	onifti(const std::string& filename,
			const inifti<Tlike, Dlike>& like,
			const unsigned int& s0,
			const unsigned int& s1,
			const unsigned int& s2):
			onifti(smt::niftiname(filename), like, s0, s1, s2) {
	}

	template <typename Tlike, unsigned int Dlike>
	onifti(const std::string& filename,
			const inifti<Tlike, Dlike>& like,
			const unsigned int& s0,
			const unsigned int& s1,
			const unsigned int& s2,
			const unsigned int& s3):
			onifti(smt::niftiname(filename), like, s0, s1, s2, s3) {
	}

	explicit operator bool() const {
		return _data != nullptr;
	}

	T& operator[](const std::size_t& ii) {
		smt::assert(0 <= ii && ii < size());
		return _data[ii];
	}

	T& operator()(const std::size_t& i0, const std::size_t& i1, const std::size_t& i2) {
		static_assert(D == 3, "D == 3");
		smt::assert(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1) && 0 <= i2 && i2 < size(2));
		return _data[i0+size(0)*(i1+size(1)*i2)];
	}

	T& operator()(const std::size_t& i0, const std::size_t& i1, const std::size_t& i2, const std::size_t& i3) {
		static_assert(D == 4, "D == 4");
		smt::assert(0 <= i0 && i0 < size(0) && 0 <= i1 && i1 < size(1) && 0 <= i2 && i2 < size(2) && 0 <= i3 && i3 < size(3));
		return _data[i0+size(0)*(i1+size(1)*(i2+size(2)*i3))];
	}

	std::size_t size() const {
		std::size_t total_size = 1;
		for(std::size_t ii = 0; ii < D; ++ii) {
			total_size *= _header.dim[ii+1];
		}
		return total_size;
	}

	std::size_t size(const std::size_t& ii) const {
		smt::assert(0 <= ii && ii < D);
		return _header.dim[ii+1];
	}

	void cal(const float& min, const float& max) {
		_header.cal_min = min;
		_header.cal_max = max;
	}

	~onifti() {
		if(_data) {
			if(_gzipped) {
				if(_separate_storage) {
#ifdef ZLIB_FOUND
					gzFile zout = gzopen(_hdrname.c_str(), "wb");
					if(zout == nullptr) {
						smt::error("Unable to open ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(gzwrite(zout, reinterpret_cast<unsigned char*>(&_header), sizeof(_header)) != sizeof(_header)) {
						smt::error("Unable to write ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(gzwrite(zout, reinterpret_cast<unsigned char*>(&_extender), sizeof(_extender)) != sizeof(_extender)) {
						smt::error("Unable to write ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(gzclose(zout) != 0) {
						smt::error("Unable to close ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}

					zout = gzopen(_imgname.c_str(), "wb");
					if(zout == nullptr) {
						smt::error("Unable to open ‘" + _imgname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(gzwrite(zout, reinterpret_cast<unsigned char*>(_data.begin()), _data.size()*sizeof(T)) != _data.size()*sizeof(T)) {
						smt::error("Unable to write ‘" + _imgname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(gzclose(zout) != 0) {
						smt::error("Unable to close ‘" + _imgname + "’.");
						std::exit(EXIT_FAILURE);
					}
#else
					smt::error("Built without support for gzip format.");
					std::exit(EXIT_FAILURE);
#endif // ZLIB_FOUND
				} else {
#ifdef ZLIB_FOUND
					gzFile zout = gzopen(_hdrname.c_str(), "wb");
					if(zout == nullptr) {
						smt::error("Unable to open ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(gzwrite(zout, reinterpret_cast<unsigned char*>(&_header), sizeof(_header)) != sizeof(_header)) {
						smt::error("Unable to write ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(gzwrite(zout, reinterpret_cast<unsigned char*>(&_extender), sizeof(_extender)) != sizeof(_extender)) {
						smt::error("Unable to write ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}

					if(gzwrite(zout, reinterpret_cast<unsigned char*>(_data.begin()), _data.size()*sizeof(T)) != _data.size()*sizeof(T)) {
						smt::error("Unable to write ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(gzclose(zout) != 0) {
						smt::error("Unable to close ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
#else
				smt::error("Built without support for gzip format.");
				std::exit(EXIT_FAILURE);
#endif // ZLIB_FOUND
				}
			} else {
				if(_separate_storage) {
					if(_mmapped) {
						if(munmap(reinterpret_cast<unsigned char*>(_data.begin())-std::max(0L, offset()), bytesize()*size()+std::max(0L, offset())) != 0) {
							smt::error("Unable to munmap ‘" + _imgname + "’.");
							std::exit(EXIT_FAILURE);
						}
						if(std::fclose(_fout) != 0) {
							smt::error("Unable to close ‘" + _imgname + "’.");
							std::exit(EXIT_FAILURE);
						}
					} else {
						if(std::fwrite(reinterpret_cast<unsigned char*>(_data.begin()), sizeof(T), _data.size(), _fout) != _data.size()) {
							smt::error("Unable to write ‘" + _imgname + "’.");
							std::exit(EXIT_FAILURE);
						}
						if(std::fclose(_fout) != 0) {
							smt::error("Unable to close ‘" + _imgname + "’.");
							std::exit(EXIT_FAILURE);
						}
					}

					_fout = fopen(_hdrname.c_str(), "wb");
					if(_fout == nullptr) {
						smt::error("Unable to open ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(fwrite(reinterpret_cast<unsigned char*>(&_header), sizeof(_header), 1u, _fout) != 1u) {
						smt::error("Unable to write ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(fwrite(reinterpret_cast<unsigned char*>(&_extender), sizeof(_extender), 1u, _fout) != 1u) {
						smt::error("Unable to write ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(fclose(_fout) != 0) {
						smt::error("Unable to close ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
				} else {
					if(fwrite(reinterpret_cast<unsigned char*>(&_header), sizeof(_header), 1u, _fout) != 1u) {
						smt::error("Unable to write ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}
					if(fwrite(reinterpret_cast<unsigned char*>(&_extender), sizeof(_extender), 1u, _fout) != 1u) {
						smt::error("Unable to write ‘" + _hdrname + "’.");
						std::exit(EXIT_FAILURE);
					}

					if(_mmapped) {
						if(munmap(reinterpret_cast<unsigned char*>(_data.begin())-std::max(352L, offset()), bytesize()*size()+std::max(352L, offset())) != 0) {
							smt::error("Unable to munmap ‘" + _hdrname + "’.");
							std::exit(EXIT_FAILURE);
						}
						if(std::fclose(_fout) != 0) {
							smt::error("Unable to close ‘" + _hdrname + "’.");
							std::exit(EXIT_FAILURE);
						}
					} else {
						if(std::fwrite(reinterpret_cast<unsigned char*>(_data.begin()), sizeof(T), _data.size(), _fout) != _data.size()) {
							smt::error("Unable to write ‘" + _hdrname + "’.");
							std::exit(EXIT_FAILURE);
						}
						if(_fout != ::stdout && std::fclose(_fout) != 0) {
							smt::error("Unable to close ‘" + _hdrname + "’.");
							std::exit(EXIT_FAILURE);
						}
					}
				}
			}
		}
	}

private:
	const bool _gzipped;
	const bool _separate_storage;
	const std::string _hdrname;
	const std::string _imgname;
	std::FILE* _fout;
	nifti_1_header _header;
	nifti1_extender _extender;
	smt::darray<T, D> _data;
	bool _mmapped;

	template <typename Tlike, unsigned int Dlike>
	onifti(const std::tuple<bool, bool, std::string, std::string>& niftiname,
			const inifti<Tlike, Dlike>& like,
			const unsigned int& s0,
			const unsigned int& s1,
			const unsigned int& s2):
			_gzipped(std::get<0>(niftiname)),
			_separate_storage(std::get<1>(niftiname)),
			_hdrname(std::get<2>(niftiname)),
			_imgname(std::get<3>(niftiname)) {
		static_assert(D == 3, "D == 3");

		_header = default_header(like._header);
		if(s0 > std::numeric_limits<signed short>::max() || s1 > std::numeric_limits<signed short>::max() || s2 > std::numeric_limits<signed short>::max()) {
			smt::error("Data size not supported by NIfTI-1 format.");
			std::exit(EXIT_FAILURE);
		}
		_header.dim[1] = s0;
		_header.dim[2] = s1;
		_header.dim[3] = s2;

		std::fill(std::begin(_extender.extension), std::end(_extender.extension), 0);

		if(_gzipped) {
#ifdef ZLIB_FOUND
			_data.resize(s0, s1, s2);
			_fout = nullptr;
			_mmapped = false;
#else
			smt::error("Built without support for gzip format.");
			std::exit(EXIT_FAILURE);
#endif // ZLIB_FOUND
		} else {
			if(_separate_storage) {
				_fout = fopen(_imgname.c_str(), "wb");
				if(_fout == nullptr) {
					smt::error("Unable to open ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}
				unsigned char* tmp = nullptr;
				if((_mmapped = ((tmp = static_cast<unsigned char*>(mmap(0, bytesize()*size()+std::max(0L, offset()), PROT_WRITE, MAP_SHARED, smt::fileno(_fout), 0))) != MAP_FAILED))) {
					_data.resize(s0, s1, s2, reinterpret_cast<T*>(tmp+bytesize()*size()+std::max(0L, offset())));
				} else {
					_data.resize(s0, s1, s2);
				}
			} else {
				_fout = (_hdrname == "-")? ::stdout : std::fopen(_hdrname.c_str(), "wb");
				if(_fout == nullptr) {
					smt::error("Unable to open ‘" + _hdrname + "’.");
					std::exit(EXIT_FAILURE);
				}
				unsigned char* tmp = nullptr;
				if((_mmapped = ((tmp = static_cast<unsigned char*>(mmap(0, bytesize()*size()+std::max(352L, offset()), PROT_WRITE, MAP_SHARED, smt::fileno(_fout), 0))) != MAP_FAILED))) {
					_data.resize(s0, s1, s2, reinterpret_cast<T*>(tmp+bytesize()*size()+std::max(352L, offset())));
				} else {
					_data.resize(s0, s1, s2);
				}
			}
		}
	}

	template <typename Tlike, unsigned int Dlike>
	onifti(const std::tuple<bool, bool, std::string, std::string>& niftiname,
			const inifti<Tlike, Dlike>& like,
			const unsigned int& s0,
			const unsigned int& s1,
			const unsigned int& s2,
			const unsigned int& s3):
			_gzipped(std::get<0>(niftiname)),
			_separate_storage(std::get<1>(niftiname)),
			_hdrname(std::get<2>(niftiname)),
			_imgname(std::get<3>(niftiname)) {
		static_assert(D == 4, "D == 4");

		_header = default_header(like._header);
		if(s0 > std::numeric_limits<signed short>::max() || s1 > std::numeric_limits<signed short>::max() || s2 > std::numeric_limits<signed short>::max() || s3 > std::numeric_limits<signed short>::max()) {
			smt::error("Data size not supported by NIfTI-1 format.");
			std::exit(EXIT_FAILURE);
		}
		_header.dim[1] = s0;
		_header.dim[2] = s1;
		_header.dim[3] = s2;
		_header.dim[4] = s3;

		std::fill(std::begin(_extender.extension), std::end(_extender.extension), 0);

		if(_gzipped) {
#ifdef ZLIB_FOUND
			_data.resize(s0, s1, s2, s3);
			_fout = nullptr;
			_mmapped = false;
#else
			smt::error("Built without support for gzip format.");
			std::exit(EXIT_FAILURE);
#endif // ZLIB_FOUND
		} else {
			if(_separate_storage) {
				_fout = fopen(_imgname.c_str(), "wb");
				if(_fout == nullptr) {
					smt::error("Unable to open ‘" + _imgname + "’.");
					std::exit(EXIT_FAILURE);
				}
				unsigned char* tmp = nullptr;
				if((_mmapped = ((tmp = static_cast<unsigned char*>(mmap(0, bytesize()*size()+std::max(0L, offset()), PROT_WRITE, MAP_SHARED, smt::fileno(_fout), 0))) != MAP_FAILED))) {
					_data.resize(s0, s1, s2, s3, reinterpret_cast<T*>(tmp+bytesize()*size()+std::max(0L, offset())));
				} else {
					_data.resize(s0, s1, s2, s3);
				}
			} else {
				_fout = (_hdrname == "-")? ::stdout : std::fopen(_hdrname.c_str(), "wb");
				if(_fout == nullptr) {
					smt::error("Unable to open ‘" + _hdrname + "’.");
					std::exit(EXIT_FAILURE);
				}
				unsigned char* tmp = nullptr;
				if((_mmapped = ((tmp = static_cast<unsigned char*>(mmap(0, bytesize()*size()+std::max(352L, offset()), PROT_WRITE, MAP_SHARED, smt::fileno(_fout), 0))) != MAP_FAILED))) {
					_data.resize(s0, s1, s2, s3, reinterpret_cast<T*>(tmp+bytesize()*size()+std::max(352L, offset())));
				} else {
					_data.resize(s0, s1, s2, s3);
				}
			}
		}
	}

	std::size_t bytesize() const {
		return nifti_bytesize(_header.datatype);
	}

	nifti_1_header default_header(const nifti_1_header& like) const {
		nifti_1_header header = like;

		header.sizeof_hdr = 348;
		std::fill(std::begin(header.data_type), std::end(header.data_type), 0); // unused
		std::fill(std::begin(header.db_name), std::end(header.db_name), 0); // unused
		header.extents = 0; // unused
		header.session_error = 0; // unused
		header.regular = 0; // unused
		header.dim[0] = D;
		std::fill(std::begin(header.dim)+1u+D, std::end(header.dim), 0);
		header.intent_p1 = 0.0f;
		header.intent_p2 = 0.0f;
		header.intent_p3 = 0.0f;
		header.intent_code = NIFTI_INTENT_NONE;
		header.datatype = nifti_datatype<T>();
		header.bitpix = 8u*sizeof(T);
		std::fill(std::begin(header.pixdim)+1u+D, std::end(header.pixdim), 0);
		if(_separate_storage) {
			header.vox_offset = 0.0f;
		} else {
			header.vox_offset = 352.0f;
		}
		header.scl_slope = 1.0f;
		header.scl_inter = 0.0f;
		header.xyzt_units = SPACE_TIME_TO_XYZT(XYZT_TO_SPACE(header.xyzt_units), NIFTI_UNITS_UNKNOWN);
		header.cal_max = 0.0f;
		header.cal_min = 0.0f;
		header.toffset = 0.0f;
		header.glmax = 0; // unused
		header.glmin = 0; // unused
		std::strncpy(header.descrip, "SMT - https://ekaden.github.io", sizeof(header.descrip));
		if(_separate_storage) {
			std::strncpy(header.magic, "ni1", sizeof(header.magic));
		} else {
			std::strncpy(header.magic, "n+1", sizeof(header.magic));
		}

		return header;
	}

	std::ptrdiff_t offset() const {
		return _header.vox_offset;
	}
};

} // smt

#endif // _NIFTI_H
