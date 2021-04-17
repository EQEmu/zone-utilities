#pragma once
#include <cstring>
#include <functional>
#include <uv.h>

namespace eqemu {
    namespace event {
        class event_loop {
        public:
            static event_loop& get() {
                static event_loop inst;
                return inst;
            }

            ~event_loop() { uv_loop_close(&_loop); }

            void process() { uv_run(&_loop, UV_RUN_NOWAIT); }

            uv_loop_t* handle() { return &_loop; }

        private:
            event_loop() {
                memset(&_loop, 0, sizeof(uv_loop_t));
                uv_loop_init(&_loop);
            }

            event_loop(const event_loop&);
            event_loop& operator=(const event_loop&);

            uv_loop_t _loop;
        };
    }    // namespace event
}    // namespace eqemu
