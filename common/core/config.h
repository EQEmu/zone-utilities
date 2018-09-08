#pragma once

#include "config_interface.h"

namespace EQEmu
{

	class Config : public IConfig {
	public:
		Config();
		virtual ~Config();

		virtual const std::string GetPath(const std::string &type, const std::string &defaultValue) const;
		virtual bool GetLogEnabled(const std::string &type, const bool defaultValue) const;
	private:
		struct Implementation;
		Implementation *mImpl;
	};
}
