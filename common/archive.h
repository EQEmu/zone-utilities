/*
	Copyright(C) 2014 EQEmu
	
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef EQEMU_COMMON_ARCHIVE_HPP
#define EQEMU_COMMON_ARCHIVE_HPP

#include <string>
#include <vector>
#include <list>

namespace EQEmu
{

class Archive
{
public:
	Archive() { }
	virtual ~Archive() { }
	
	virtual bool Open(std::string filename) = 0;
	virtual bool Close() = 0;
	virtual bool Get(std::string filename, std::vector<char> &buffer) = 0;
	virtual bool Exists(std::string filename) = 0;
	virtual bool GetFilenames(std::string extension, std::list<std::string> &files) = 0;
};

}

#endif
