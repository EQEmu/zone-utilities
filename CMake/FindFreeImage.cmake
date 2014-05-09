#CMake - Cross Platform Makefile Generator
#Copyright 2000-2011 Kitware, Inc., Insight Software Consortium
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions
#are met:
#
#* Redistributions of source code must retain the above copyright
#  notice, this list of conditions and the following disclaimer.
#
#* Redistributions in binary form must reproduce the above copyright
#  notice, this list of conditions and the following disclaimer in the
#  documentation and/or other materials provided with the distribution.
#
#* Neither the names of Kitware, Inc., the Insight Software Consortium,
#  nor the names of their contributors may be used to endorse or promote
#  products derived from this software without specific prior written
#  permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This module defines
#  FREEIMAGE_FOUND, if false, do not try to link to FreeImage
#  FREEIMAGE_LIBRARIES
#  FREEIMAGE_INCLUDE_DIR, where to find FreeImage.h

IF(FREEIMAGE_ROOT)
	FIND_PATH(FREEIMAGE_INCLUDE_DIR
		NAMES FreeImage.h
		HINTS
			ENV FREEIMAGE_DIR
		PATHS
		${FREEIMAGE_ROOT}
		~/Library/Frameworks
		/Library/Frameworks
		/sw
		/opt/local
		/opt/csw
		/opt
		PATH_SUFFIXES include/FreeImage include src
	)
	
	FIND_LIBRARY(FREEIMAGE_LIBRARY
		NAMES FreeImage
		HINTS
			ENV FREEIMAGE_DIR
		PATHS
		${FREEIMAGE_ROOT}
		~/Library/Frameworks
		/Library/Frameworks
		/sw
		/opt/local
		/opt/csw
		/opt
		PATH_SUFFIXES lib bin
	)
ELSE(FREEIMAGE_ROOT)
	FIND_PATH(FREEIMAGE_INCLUDE_DIR
		NAMES FreeImage.h
		HINTS
			ENV FREEIMAGE_DIR
		PATHS
		~/Library/Frameworks
		/Library/Frameworks
		/sw
		/opt/local
		/opt/csw
		/opt
		PATH_SUFFIXES include/FreeImage include
	)
	
	FIND_LIBRARY(FREEIMAGE_LIBRARY
		NAMES FreeImage
		HINTS
			ENV FREEIMAGE_DIR
		PATHS
		~/Library/Frameworks
		/Library/Frameworks
		/sw
		/opt/local
		/opt/csw
		/opt
		PATH_SUFFIXES lib bin
	)
ENDIF(FREEIMAGE_ROOT)

IF(FREEIMAGE_LIBRARY)
	SET( FREEIMAGE_LIBRARIES "${FREEIMAGE_LIBRARY}" CACHE STRING "FreeImage Libraries")
ENDIF()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FreeImage
                                  REQUIRED_VARS FREEIMAGE_LIBRARIES FREEIMAGE_INCLUDE_DIR)

MARK_AS_ADVANCED(FREEIMAGE_INCLUDE_DIR FREEIMAGE_LIBRARIES FREEIMAGE_LIBRARY FREEIMAGE_MATH_LIBRARY)


