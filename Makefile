cmake:
	git submodule init
	git submodule update --init --recursive
	mkdir -p build
	cd build && cmake ..
.PHONY: build
build:
	cd build && make