#ifndef EQEMU_COMMON_CONFIG_H
#define EQEMU_COMMON_CONFIG_H

#include <string>

namespace eqemu
{
	class config {
	public:
		~config();

		static config& Instance() {
			static config inst;
			return inst;
		}

		const std::string get_path(const std::string &type, const std::string &defaultValue);

	private:
		config();
		config(const config&);
		config& operator=(const config&);

		struct implementation;
		implementation *_impl;
	};
}

#endif
