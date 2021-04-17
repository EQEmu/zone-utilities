#pragma once
#include "event_loop.h"
#include <functional>

namespace eqemu {
    namespace event {
        class timer {
        public:
            timer(std::function<void(timer*)> cb) {
                _timer = nullptr;
                _cb = cb;
            }

            timer(uint64_t duration_ms, bool repeats, std::function<void(timer*)> cb) {
                _timer = nullptr;
                _cb = cb;
                start(duration_ms, repeats);
            }

            ~timer() { stop(); }

            void start(uint64_t duration_ms, bool repeats) {
                auto loop = event_loop::get().handle();
                if(!_timer) {
                    _timer = new uv_timer_t;
                    memset(_timer, 0, sizeof(uv_timer_t));
                    uv_timer_init(loop, _timer);
                    _timer->data = this;

                    if(repeats) {
                        uv_timer_start(
                            _timer,
                            [](uv_timer_t* handle) {
                                timer* t = (timer*)handle->data;
                                t->_exec();
                            },
                            duration_ms,
                            duration_ms);
                    } else {
                        uv_timer_start(
                            _timer,
                            [](uv_timer_t* handle) {
                                timer* t = (timer*)handle->data;
                                t->stop();
                                t->_exec();
                            },
                            duration_ms,
                            0);
                    }
                }
            }

            void stop() {
                if(_timer) {
                    uv_close((uv_handle_t*)_timer, [](uv_handle_t* handle) { delete handle; });
                    _timer = nullptr;
                }
            }

        private:
            void _exec() { _cb(this); }

            uv_timer_t* _timer;
            std::function<void(timer*)> _cb;
        };
    }    // namespace event
}    // namespace eqemu
