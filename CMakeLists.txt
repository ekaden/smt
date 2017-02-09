# Copyright (c) 2016 Enrico Kaden & University College London
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 2.8)

project(smt)

# GNU C/C++ compiler
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_C_COMPILER_VERSION VERSION_LESS 4.9)
		message(FATAL_ERROR "gcc version must be at least 4.9")
	else()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast")
	endif()
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
		message(FATAL_ERROR "g++ version must be at least 4.9")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Ofast")
	endif()
endif()

# Clang C/C++ compiler
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
	if(CMAKE_C_COMPILER_VERSION VERSION_LESS 3.5)
		message(FATAL_ERROR "clang version must be at least 3.5")
	else()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast")
	endif()
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5)
		message(FATAL_ERROR "clang++ version must be at least 3.5")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Ofast")
	endif()
endif()

# Intel C/C++ compiler
if(CMAKE_C_COMPILER_ID STREQUAL "Intel")
	if(CMAKE_C_COMPILER_VERSION VERSION_LESS 16.0)
		message(FATAL_ERROR "icc version must be at least 16.0")
	else()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -gcc-name=gcc-4.9 -diag-disable 161")
	endif()
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16.0)
		message(FATAL_ERROR "icpc version must be at least 16.0")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Ofast -gxx-name=g++-4.9 -diag-disable 161")
	endif()
endif()

find_package(Git)
if(GIT_FOUND)
	execute_process(
			COMMAND ${GIT_EXECUTABLE} describe --tags
			OUTPUT_STRIP_TRAILING_WHITESPACE
			OUTPUT_VARIABLE SMT_VERSION_STRING
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
	add_definitions(-DSMT_VERSION_STRING=${SMT_VERSION_STRING})
endif()

find_package(OpenMP)
if(OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

find_package(Threads REQUIRED)

find_package(ZLIB)
if(ZLIB_FOUND)
	add_definitions(-DZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
endif()

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/docopt.cpp/include")
include_directories("${PROJECT_SOURCE_DIR}/niftilib/include")

add_library(docopt STATIC docopt.cpp/src/docopt.cpp)

add_executable(gaussianfit src/gaussianfit.cpp)
target_link_libraries(gaussianfit docopt)

add_executable(ricianfit src/ricianfit.cpp)
target_link_libraries(ricianfit docopt)

add_executable(fitmicrodt src/fitmicrodt.cpp)
target_link_libraries(fitmicrodt docopt)

add_executable(fitmcmicro src/fitmcmicro.cpp)
target_link_libraries(fitmcmicro docopt)

if(ZLIB_FOUND)
	target_link_libraries(gaussianfit ${ZLIB_LIBRARIES})
	target_link_libraries(ricianfit ${ZLIB_LIBRARIES})
	target_link_libraries(fitmicrodt ${ZLIB_LIBRARIES})
	target_link_libraries(fitmcmicro ${ZLIB_LIBRARIES})
endif()

install(TARGETS gaussianfit ricianfit fitmicrodt fitmcmicro DESTINATION bin)
install(FILES README.md LICENSE.md THIRDPARTY.md DESTINATION .)

if(GIT_FOUND)
	string(REGEX REPLACE "^v" "" SMT_PACKAGE_VERSION ${SMT_VERSION_STRING})
else()
	set(SMT_PACKAGE_VERSION "unknown")
endif()

set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "${SMT_PACKAGE_VERSION}")
set(CPACK_GENERATOR "TGZ;ZIP")

include(CPack)
