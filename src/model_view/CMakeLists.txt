CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

SET(model_view_sources
	main.cpp
)

SET(model_view_headers
)

ADD_EXECUTABLE(model_view ${model_view_sources} ${model_view_headers})

INSTALL(TARGETS model_view RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(model_view PRIVATE common)
TARGET_LINK_LIBRARIES(model_view PRIVATE log)
TARGET_LINK_LIBRARIES(model_view PRIVATE imgui)
TARGET_LINK_LIBRARIES(model_view PRIVATE glfw)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
