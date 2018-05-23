CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

SET(imgui_sources
	imgui.cpp
	imgui_draw.cpp
)

SET(imgui_headers
	imconfig.h
	imgui.h
	stb_rect_pack.h
	stb_textedit.h
	stb_truetype.h
)

ADD_LIBRARY(imgui ${imgui_sources} ${imgui_headers})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
