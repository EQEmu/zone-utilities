#pragma once

#include <string>

namespace eqemu {
    namespace core {
        class config {
        public:
            ~config();

            static config& instance() {
                static config inst;
                return inst;
            }

            const std::string get_path(const std::string& type, const std::string& default_value);

        private:
            config();
            config(const config&);
            config& operator=(const config&);

            struct implementation;
            implementation* _impl;
        };
    }    // namespace core
}    // namespace eqemu
