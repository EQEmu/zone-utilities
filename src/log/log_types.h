#ifndef EQEMU_LOG_LOG_TYPES_H
#define EQEMU_LOG_LOG_TYPES_H

namespace EQEmu {

    namespace Log {

        enum LogType {
            LogTrace = 1,
            LogDebug = 2,
            LogInfo = 4,
            LogWarn = 8,
            LogError = 16,
            LogFatal = 32,
            LogAll = 63
        };

    }

}    // namespace EQEmu

#endif
