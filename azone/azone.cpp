#include <stdio.h>
#include "map.h"
#include "pfs.h"

int main(int argc, char **argv) {

	for(int i = 1; i < argc; ++i) {
		Map m;
		if(!m.Build(argv[i])) {
			printf("Failed to build map for zone: %s\n", argv[i]);
		} else {
			m.Write(std::string(argv[i]) + std::string(".map"));
		}
	}

	getchar();
	return 0;
}
