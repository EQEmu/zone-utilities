#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "map.h"

int main(int argc, char **argv) {
	Map *m = Map::LoadMapFile("nektulos");

	if(!m)
	{
		printf("Couldn't load map =(\n");
	}

	Map::Vertex loc(0.0f, 0.0f, 0.0f);
	float best_z = m->FindBestZ(loc, nullptr);
	if(best_z > BEST_Z_INVALID) {
		printf("Best_z: %f\n", best_z);
	} else {
		printf("Could not find best_z!\n");
	}

	return 0;
}
