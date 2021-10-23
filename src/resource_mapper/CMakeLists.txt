CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

SET(resource_mapper_sources
	main.cpp
)

SET(resource_mapper_headers
)

ADD_EXECUTABLE(resource_mapper ${resource_mapper_sources} ${resource_mapper_headers})

INSTALL(TARGETS resource_mapper RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(resource_mapper PRIVATE common)
TARGET_LINK_LIBRARIES(resource_mapper PRIVATE fmt::fmt)
TARGET_LINK_LIBRARIES(resource_mapper PRIVATE log)
TARGET_LINK_LIBRARIES(resource_mapper PRIVATE ZLIB::ZLIB)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)