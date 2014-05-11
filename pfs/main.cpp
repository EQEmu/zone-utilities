#include <stdio.h>
#include <string.h>
#include "pfs.h"

int main(int argc, char **argv) {
	if(argc < 3) {
		printf("Usage: pfs [-l filename [ext]] [-e filename]\n\n");
		printf("Options:\n");
		printf("\t-l filename ext    List all files by optional file extension or wildcard.\n");
		printf("\t-e filename        Extract a file from the archive.\n");
		return 0;
	}
	
	if(strcmp(argv[1], "-l") == 0) {
		std::string ext;
		if (argc < 4) {
			ext = "*";
		} else {
			ext = argv[3];
		}

		EQEmu::PFS::Archive archive;
		if(!archive.Open(argv[2])) {
			printf("Unable to open %s\n", argv[2]);
			return 0;
		}

		std::list<std::string> files;
		archive.GetFilenames(ext, files);

		printf("Files with extension %s in %s:\n", ext.c_str(), argv[2]);
		auto iter = files.begin();
		while (iter != files.end()) {
			printf("%s\n", (*iter).c_str());
			++iter;
		}

	} else if(strcmp(argv[1], "-e") == 0) {
		EQEmu::PFS::Archive archive;
		if (!archive.Open(argv[2])) {
			printf("Unable to open %s\n", argv[2]);
			return 0;
		}

		std::vector<char> buffer;
		if (archive.Get(argv[3], buffer)) {
			FILE *f = fopen(argv[3], "wb");
			if(f) {
				if (fwrite(&buffer[0], buffer.size(), 1, f) != 1) {
					fclose(f);
					printf("Could not write file %s\n", argv[3]);
					return 0;
				}
				fclose(f);
			} else {
				printf("Could not write file %s\n", argv[3]);
				return 0;
			}
		}
	} else {
		printf("Usage: pfs [-l filename [ext]] [-e filename]\n\n");
		printf("Options:\n");
		printf("\t-l filename ext    List all files by optional file extension or wildcard.\n");
		printf("\t-e filename        Extract a file from the archive.\n");
	}
	
	return 0;
}
