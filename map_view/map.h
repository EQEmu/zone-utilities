#ifndef EQEMU_MAP_H
#define EQEMU_MAP_H

#include "model.h"

void LoadMap(std::string filename, Model **collision, Model **liquid, Model **vision);

#endif
