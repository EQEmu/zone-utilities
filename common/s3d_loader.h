#ifndef EQEMU_S3D_LOADER_H
#define EQEMU_S3D_LOADER_H

#include <vector>
#include <stdint.h>
#include <string>
#include "wld_fragment.h"

void decode_string_hash(char *str, size_t len);

namespace EQEmu
{

class S3DLoader
{
public:
	S3DLoader();
	~S3DLoader();
	bool Load(std::string zone_name,
		std::vector<WLDFragment> &zone_frags,
		std::vector<WLDFragment> &zone_object_frags,
		std::vector<WLDFragment> &zone_light_frags,
		std::vector<WLDFragment> &object_frags,
		std::vector<WLDFragment> &object2_frags,
		std::vector<WLDFragment> &character_frags);
private:
	bool ParseWLDFile(std::string file_name, std::string wld_name, std::vector<WLDFragment> &out);
};

}

#endif
