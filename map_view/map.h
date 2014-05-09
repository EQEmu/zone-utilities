#ifndef EQEMU_MAP_H
#define EQEMU_MAP_H

#include "model.h"
#include <string>

void LoadMap(std::string filename, Model **collision, Model **vision);

#endif
