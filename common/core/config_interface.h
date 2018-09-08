#pragma once

#include <string>

namespace EQEmu
{
	class IConfig {
	public:
		IConfig() { }
		virtual ~IConfig() { }

		virtual const std::string GetPath(const std::string &type, const std::string &defaultValue) const = 0;
		virtual bool GetLogEnabled(const std::string &type, const bool defaultValue) const = 0;
	};
}
