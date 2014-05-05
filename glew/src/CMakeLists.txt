CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(glew_sources
	glew.c
	glewinfo.c
	visualinfo.c
)

SET(glew_headers
)

ADD_LIBRARY(glew ${glew_sources} ${glew_headers})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)