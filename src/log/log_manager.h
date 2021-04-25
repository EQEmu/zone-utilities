#ifndef EQEMU_LOG_LOG_MANAGER_H
#define EQEMU_LOG_LOG_MANAGER_H

#include "log_base.h"
#include <memory>
#include <vector>

namespace EQEmu {

    namespace Log {

        class Manager {
        public:
            ~Manager();
            static void Init(int enabled_logs);
            static Manager& Instance();

            void RegisterLog(std::shared_ptr<LogBase> log);
            void Log(LogType type, const char* str, ...);

        private:
            Manager() {}
            Manager(const Manager& s);
            const Manager& operator=(const Manager& s);

            void DispatchMessage(LogType type, const std::string& message);

            int enabled_logs;
            std::vector<std::shared_ptr<LogBase>> logs;
        };

    }    // namespace Log

}    // namespace EQEmu

#endif
