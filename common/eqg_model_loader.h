#ifndef EQEMU_COMMON_EQG_MODEL_LOADER_H
#define EQEMU_COMMON_EQG_MODEL_LOADER_H

#include <stdint.h>
#include <memory>
#include "pfs.h"
#include "eqg_geometry.h"

namespace EQEmu
{

class EQGModelLoader
{
public:
	EQGModelLoader();
	~EQGModelLoader();
	bool Load(EQEmu::PFS::Archive &archive, std::string model, std::shared_ptr<EQG::Geometry> model_out);
};

}

#endif
