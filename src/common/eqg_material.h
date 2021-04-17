#ifndef EQEMU_COMMON_EQG_MATERIAL_H
#define EQEMU_COMMON_EQG_MATERIAL_H

#include <stdint.h>
#include <string>
#include <vector>

namespace EQEmu {

    namespace EQG {

        class Material {
        public:
            struct Property {
                std::string name;
                uint32_t type;
                int32_t value_i;
                float value_f;
                std::string value_s;
            };

            Material() {}
            ~Material() {}

            void SetName(std::string n) { name = n; }
            void SetShader(std::string s) { shader = s; }

            std::string& GetName() { return name; }
            std::string& GetShader() { return shader; }
            std::vector<Property>& GetProperties() { return properties; }

        private:
            std::string name;
            std::string shader;
            std::vector<Property> properties;
        };

    }    // namespace EQG

}    // namespace EQEmu

#endif
