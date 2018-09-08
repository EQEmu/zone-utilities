#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "eq/pfs.h"

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
	printf("Usage: pfs [<switches>...] <command> <command args>... <archive_name> [<file_names>...]\n"
	"<Switches>\n"
	" -i=dir: Set input directory\n"
	" -o=dir: Set output directory\n"
	"<Commands>\n"
	" a: Add files to archive\n"
	" d: Delete files from the archive\n"
	" e: Extract files from the archive\n"
	" l: List contents of the archive\n"
	" <Command Args>\n"
	"  arg1: Only search for files with this extension, may use * as a wildcard meaning all extensions\n"
	" u: Update files of the archive\n"
	);
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

	int argi = 1;

	std::string input_dir = ".";
	std::string output_dir = ".";

	while (strlen(argv[argi]) > 3 && argv[argi][0] == '-') {
		if (argv[argi][1] == 'i' && argv[argi][2] == '=') {
			size_t str_sz = strlen(argv[argi]) - 3;
			if(str_sz == 0) {
				PrintUsage();
				return EXIT_FAILURE;
			}

			input_dir.resize(str_sz);
			memcpy(&input_dir[0], &argv[argi][3], str_sz);
		} else if (argv[argi][1] == 'o' && argv[argi][2] == '=') {
			size_t str_sz = strlen(argv[argi]) - 3;
			if (str_sz == 0) {
				PrintUsage();
				return EXIT_FAILURE;
			}

			output_dir.resize(str_sz);
			memcpy(&output_dir[0], &argv[argi][3], str_sz);
		} else {
			PrintUsage();
			return EXIT_FAILURE;
		}

		argi++;
	}


	CommandType current_command = CommandUnknown;
	if (strcmp(argv[argi], "a") == 0) {
		current_command = CommandAdd;
	}
	else if (strcmp(argv[argi], "d") == 0) {
		current_command = CommandDelete;
	}
	else if (strcmp(argv[argi], "e") == 0) {
		current_command = CommandExtract;
	}
	else if (strcmp(argv[argi], "l") == 0) {
		current_command = CommandList;
	}
	else if (strcmp(argv[argi], "u") == 0) {
		current_command = CommandUpdate;
	}

	if(current_command == CommandUnknown) {
		printf("Invalid command argument %s\n", argv[argi]);
		PrintUsage();
		return EXIT_FAILURE;
	}

	argi++;

	std::string input_file;
	std::string output_file;
	std::vector<std::string> files;
	if(current_command == CommandList) {
		std::string ext;
		if (argc < argi + 1) {
			PrintUsage();
			return EXIT_FAILURE;
		}

		ext = argv[argi++];

		if (argc < argi + 1) {
			PrintUsage();
			return EXIT_FAILURE;
		}

		input_file = argv[argi++];
		input_file = input_dir + "/" + input_file;

		EQEmu::PFS::Archive archive;
		if (!archive.Open(input_file)) {
			printf("Unable to open archive %s\n", input_file.c_str());
			return EXIT_FAILURE;
		}
		
		std::vector<std::string> files;
		archive.GetFilenames(ext, files);
		
		printf("Files with extension %s in %s:\n", ext.c_str(), input_file.c_str());
		for(uint32_t i = 0; i < files.size(); ++i) {
			printf("%s\n", files[i].c_str());
		}

		return EXIT_SUCCESS;
	} else {
		if (argc < argi + 1) {
			PrintUsage();
			return EXIT_FAILURE;
		}

		input_file = argv[argi++];
		output_file = output_dir + "/" + input_file;
		input_file = input_dir + "/" + input_file;

		for (int i = argi; i < argc; ++i) {
			files.push_back(argv[i]);
		}
	}

	EQEmu::PFS::Archive archive;
	if (!archive.Open(input_file)) {
		archive.Open();
	}

	if(current_command == CommandAdd) {
		std::vector<char> current_file;
		for(size_t i = 0; i < files.size(); ++i) {
			if (archive.Exists(files[i])) {
				printf("Warning: Could not add %s to the archive, file with that name already exists.\n", files[i].c_str());
				continue;
			}

			if(!ReadFile(files[i], current_file)) {
				printf("Warning: Could not find %s to add to archive.\n", files[i].c_str());
				continue;
			} else {
				archive.Set(files[i], current_file);
			}
		}
	} else if(current_command == CommandDelete) {
		for (size_t i = 0; i < files.size(); ++i) {
			if (!archive.Delete(files[i])) {
				printf("Warning: Could not delete %s from the archive.\n", files[i].c_str());
			}
		}
	} else if (current_command == CommandExtract) {
		std::vector<char> current_file;
		for (size_t i = 0; i < files.size(); ++i) {
			if (!archive.Exists(files[i])) {
				printf("Warning: Could not extract %s from the archive, file with that name does not exist.\n", files[i].c_str());
				continue;
			}

			if(!archive.Get(files[i], current_file)) {
				printf("Warning: Could not extract %s from the archive, could not find file in archive.\n", files[i].c_str());
				continue;
			}

			std::string filename_out = output_dir + "/" + files[i];

			if (!WriteFile(filename_out, current_file)) {
				printf("Warning: Could not extract %s from the archive, could not write file to output directory.\n", files[i].c_str());
				continue;
			}
		}

		return EXIT_SUCCESS;
	} else if (current_command == CommandUpdate) {
		std::vector<char> current_file;
		for (size_t i = 0; i < files.size(); ++i) {
			if (!archive.Exists(files[i])) {
				printf("Warning: Could not update %s in the archive, file with that name does not exist.\n", files[i].c_str());
				continue;
			}

			if (!ReadFile(files[i], current_file)) {
				printf("Warning: Could not find %s to update in archive.\n", files[i].c_str());
				continue;
			}
			else {
				archive.Set(files[i], current_file);
			}
		}
	}

	if (!archive.Save(output_file)) {
		printf("Error: Could not save archive to %s\n", output_file.c_str());
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
