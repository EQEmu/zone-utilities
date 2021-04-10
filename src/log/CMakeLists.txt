CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

SET(log_sources
	log_file.cpp
	log_manager.cpp
	log_stdout.cpp
)

SET(log_headers
	log_base.h
	log_file.h
	log_macros.h
	log_manager.h
	log_stdout.h
	log_types.h
)

ADD_LIBRARY(log ${log_sources} ${log_headers})


SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
