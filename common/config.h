#ifndef EQEMU_COMMON_CONFIG_H
#define EQEMU_COMMON_CONFIG_H

#include <string>

class Config {
public:
	~Config();

	static Config& Instance() {
		static Config inst;
		return inst;
	}

	const std::string GetPath(const std::string &type, const std::string &defaultValue);

private:
	Config();
	Config(const Config&);
	Config& operator=(const Config&);

	struct Implementation;
	Implementation *mImpl;
};

#endif
