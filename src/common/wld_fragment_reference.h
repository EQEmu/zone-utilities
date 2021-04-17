#ifndef EQEMU_COMMON_WLD_FRAGMENT_REFERENCE_H
#define EQEMU_COMMON_WLD_FRAGMENT_REFERENCE_H

#include "wld_fragment.h"
#include <vector>

namespace EQEmu {

    class WLDFragmentReference {
    public:
        WLDFragmentReference() {}
        ~WLDFragmentReference() {}

        void SetName(std::string nname) { name = nname; }
        void SetMagicString(std::string nstr) { magic_str = nstr; }

        std::vector<uint32_t>& GetFrags() { return frags; }
        std::string& GetName() { return name; }
        std::string& GetMagicString() { return name; }

    private:
        std::vector<uint32_t> frags;
        std::string name;
        std::string magic_str;
    };

}    // namespace EQEmu

#endif
