#ifndef EQEMU_EQG_MODEL_LOADER_H
#define EQEMU_EQG_MODEL_LOADER_H

#include <vector>
#include <stdint.h>
#include <memory>
#include "pfs.h"
#include "eqg_geometry.h"

class EQGModelLoader
{
public:
	EQGModelLoader();
	~EQGModelLoader();
	bool Load(EQEmu::PFS::Archive &archive, std::string model, std::shared_ptr<EQG::Geometry> model_out);
};

#endif
