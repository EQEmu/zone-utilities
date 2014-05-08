#ifndef EQEMU_EQG_MATERIAL_H
#define EQEMU_EQG_MATERIAL_H

#include <string>
#include <vector>
#include <stdint.h>

namespace EQG
{

class Material
{
public:
	struct Property
	{
		std::string name;
		uint32_t type;
		int32_t value_i;
		float value_f;
		std::string value_s;
	};

	Material() { }
	~Material() { }

	void SetName(std::string n) { name = n; }
	void SetShader(std::string s) { shader = s; }
	void AddProperty(Property &p) { properties.push_back(p); }
	
	std::string &GetName() { return name; }
	std::string &GetShader() { return shader; }
	std::vector<Property> &GetProperties() { return properties; }
private:
	std::string name;
	std::string shader;
	std::vector<Property> properties;
};

}

#endif
