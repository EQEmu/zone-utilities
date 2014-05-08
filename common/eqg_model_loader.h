#ifndef EQEMU_EQG_MODEL_LOADER_H
#define EQEMU_EQG_MODEL_LOADER_H

#include <vector>
#include <stdint.h>
#include "pfs.h"
#include "eqg_material.h"

class EQGModelLoader
{
public:
	EQGModelLoader();
	~EQGModelLoader();
	bool Load(EQEmu::PFS::Archive &archive, std::string model);
};

#endif
