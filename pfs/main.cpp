#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pfs.h"

enum CommandType
{
	CommandUnknown,
	CommandAdd,
	CommandDelete,
	CommandExtract,
	CommandList,
	CommandUpdate
};

void PrintUsage() {
	printf("Usage: pfs <command> <command args>... <archive_name> [<file_names>...]\n"
	"<Commands>\n"
	" a: Add files to archive\n"
	" d: Delete files from the archive\n"
	" e: Extract files from the archive\n"
	" l: List contents of the archive\n"
	" <Command Args>\n"
	"  arg1: Only search for files with this extension, may use * as a wildcard meaning all extensions\n"
	" u: Update files of the archive\n");
}

bool ReadFile(std::string filename, std::vector<char> &buffer) {
	FILE *f = fopen(filename.c_str(), "rb");
	if(f) {
		fseek(f, 0, SEEK_END);
		size_t sz = ftell(f);
		rewind(f);

		buffer.resize(sz);
		size_t res = fread(&buffer[0], 1, sz, f);
		if (res != sz) {
			return false;
		}

		fclose(f);
		return true;
	}

	return false;
}

bool WriteFile(std::string filename, const std::vector<char> &buffer) {
	FILE *f = fopen(filename.c_str(), "wb");
	if (f) {
		size_t res = fwrite(&buffer[0], buffer.size(), 1, f);
		if(res != 1) {
			fclose(f);
			return false;
		}

		fclose(f);
		return true;
	}

	return false;
}

int main(int argc, char **argv) {

	if(argc < 2) {
		PrintUsage();
		return EXIT_FAILURE;
	}

	CommandType current_command = CommandUnknown;
	if(strcmp(argv[1], "a") == 0) {
		current_command = CommandAdd;
	}
	else if (strcmp(argv[1], "d") == 0) {
		current_command = CommandDelete;
	}
	else if (strcmp(argv[1], "e") == 0) {
		current_command = CommandExtract;
	}
	else if (strcmp(argv[1], "l") == 0) {
		current_command = CommandList;
	}
	else if (strcmp(argv[1], "u") == 0) {
		current_command = CommandUpdate;
	}

	if(current_command == CommandUnknown) {
		printf("Invalid command argument %s\n", argv[1]);
		PrintUsage();
		return EXIT_FAILURE;
	}

	std::string archive_file;
	std::vector<std::string> files;
	if(current_command == CommandList) {
		std::string ext;
		if (argc < 3) {
			PrintUsage();
			return EXIT_FAILURE;
		}

		ext = argv[2];

		if (argc < 4) {
			PrintUsage();
			return EXIT_FAILURE;
		}

		archive_file = argv[3];

		EQEmu::PFS::Archive archive;
		if (!archive.Open(archive_file)) {
			printf("Unable to open archive %s\n", archive_file.c_str());
			return 0;
		}
		
		std::vector<std::string> files;
		archive.GetFilenames(ext, files);
		
		printf("Files with extension %s in %s:\n", ext.c_str(), archive_file.c_str());
		for(uint32_t i = 0; i < files.size(); ++i) {
			printf("%s\n", files[i].c_str());
		}

		return EXIT_SUCCESS;
	} else {
		if (argc < 3) {
			PrintUsage();
			return EXIT_FAILURE;
		}

		archive_file = argv[2];

		for(int i = 3; i < argc; ++i) {
			files.push_back(argv[i]);
		}
	}

	EQEmu::PFS::Archive archive;
	if (!archive.Open(archive_file)) {
		archive.Open();
	}

	if(current_command == CommandAdd) {
		std::vector<char> current_file;
		for(size_t i = 0; i < files.size(); ++i) {
			if (archive.Exists(files[i])) {
				printf("Warning: Could not add %s to the archive %s, file with that name already exists.\n", files[i].c_str(), archive_file.c_str());
				continue;
			}

			if(!ReadFile(files[i], current_file)) {
				printf("Warning: Could not find %s to add to archive %s.\n", files[i].c_str(), archive_file.c_str());
				continue;
			} else {
				archive.Set(files[i], current_file);
			}
		}
	} else if(current_command == CommandDelete) {
		for (size_t i = 0; i < files.size(); ++i) {
			if (!archive.Delete(files[i])) {
				printf("Warning: Could not delete %s from the archive %s.\n", files[i].c_str(), archive_file.c_str());
			}
		}
	} else if (current_command == CommandExtract) {
		std::vector<char> current_file;
		for (size_t i = 0; i < files.size(); ++i) {
			if (!archive.Exists(files[i])) {
				printf("Warning: Could not extract %s from the archive %s, file with that name does not exist.\n", files[i].c_str(), archive_file.c_str());
				continue;
			}

			if(!archive.Get(files[i], current_file)) {
				printf("Warning: Could not extract %s from the archive %s, could not find file in archive.\n", files[i].c_str(), archive_file.c_str());
				continue;
			}

			if (!WriteFile(files[i], current_file)) {
				printf("Warning: Could not extract %s from the archive %s, could not write file to output directory.\n", files[i].c_str(), archive_file.c_str());
				continue;
			}
		}
	} else if (current_command == CommandUpdate) {
		std::vector<char> current_file;
		for (size_t i = 0; i < files.size(); ++i) {
			if (!archive.Exists(files[i])) {
				printf("Warning: Could not update %s in the archive %s, file with that name does not exist.\n", files[i].c_str(), archive_file.c_str());
				continue;
			}

			if (!ReadFile(files[i], current_file)) {
				printf("Warning: Could not find %s to update in archive %s.\n", files[i].c_str(), archive_file.c_str());
				continue;
			}
			else {
				archive.Set(files[i], current_file);
			}
		}
	}

	if (!archive.Save(archive_file)) {
		printf("Error: Could not save archive to %s\n", archive_file.c_str());
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
