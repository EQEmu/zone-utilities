#include <stdio.h>
#include "water_map.h"

int main(int argc, char **argv) {

	for(int i = 1; i < argc; ++i) {
		WaterMap m;
		if(!m.BuildAndWrite(argv[i])) {
			printf("Failed to build and write water map for zone: %s\n", argv[i]);
		}
	}

	getchar();
	return 0;
}
