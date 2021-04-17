#pragma once
#include "event_loop.h"
#include <functional>

namespace eqemu {
    namespace event {
        class background_task {
        public:
            typedef std::function<void(void)> background_task_function;
            struct background_task_baton {
                background_task_function fn;
                background_task_function on_finish;
            };

            background_task(background_task_function fn, background_task_function on_finish) {
                uv_work_t* m_work = new uv_work_t;
                memset(m_work, 0, sizeof(uv_work_t));
                background_task_baton* baton = new background_task_baton();
                baton->fn = fn;
                baton->on_finish = on_finish;

                m_work->data = baton;
                uv_queue_work(
                    event_loop::get().handle(),
                    m_work,
                    [](uv_work_t* req) {
                        background_task_baton* baton = (background_task_baton*)req->data;
                        baton->fn();
                    },
                    [](uv_work_t* req, int status) {
                        background_task_baton* baton = (background_task_baton*)req->data;
                        baton->on_finish();
                        delete baton;
                        delete req;
                    });
            }

            ~background_task() {}
        };
    }    // namespace event
}    // namespace eqemu
