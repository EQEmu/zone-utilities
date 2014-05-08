#ifndef EQEMU_EQG_LOADER_H
#define EQEMU_EQG_LOADER_H

#include <vector>
#include <stdint.h>
#include <string>
#include "eqg_geometry.h"

class EQGLoader
{
public:
	EQGLoader();
	~EQGLoader();
	bool Load(std::string file);
private:
	bool GetZon(std::string file, std::vector<char> &buffer);
	bool ParseZon(std::vector<char> &buffer);
	bool EQGLoader::LoadModel(std::string file);
};

#endif
