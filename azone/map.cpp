#include "map.h"
#include "s3d_loader.h"

Map::Map() {
}

Map::~Map() {
}

bool Map::Build(std::string zone_name) {
	S3DLoader s3d;
	std::vector<WLDFragment> zone_frags;
	std::vector<WLDFragment> zone_object_frags;
	std::vector<WLDFragment> zone_light_frags;
	std::vector<WLDFragment> object_frags;
	std::vector<WLDFragment> character_frags;
	if(s3d.Load(zone_name, zone_frags, zone_object_frags, zone_light_frags, object_frags, character_frags)) {
		//process frags here
		return true;
	}

	//try to load a v1-3 eqg here

	//if that fails try to load a v4 eqg here

	//all hath failed
	return false;
}

bool Map::Write(std::string filename) {
	return false;
}
