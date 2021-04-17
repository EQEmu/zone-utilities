#pragma once
#include <exception>
#include <functional>

#include "event_loop.h"

namespace eqemu {
    namespace event {
        struct task_void_error {};

        template <typename Ty, typename ErrTy = task_void_error>
        class task {
        public:
            typedef std::function<void(const Ty&)> then_fn;
            typedef std::function<void(const ErrTy&)> error_fn;
            typedef std::function<void()> finish_fn;
            typedef std::function<void(then_fn, error_fn)> task_fn;

            struct task_baton {
                task_fn fn;
                then_fn on_then;
                error_fn on_catch;
                finish_fn on_finish;
                bool has_result;
                Ty result;
                bool has_error;
                ErrTy error;
            };

            task(task_fn fn) { _fn = fn; }

            ~task() {}

            task& then(then_fn fn) {
                _then = fn;
                return *this;
            }

            task& error(error_fn fn) {
                _error = fn;
                return *this;
            }

            task& finally(finish_fn fn) {
                _finish = fn;
                return *this;
            }

            void run() {
                uv_work_t* m_work = new uv_work_t;
                memset(m_work, 0, sizeof(uv_work_t));
                task_baton* baton = new task_baton();
                baton->fn = _fn;
                baton->on_then = _then;
                baton->on_catch = _error;
                baton->on_finish = _finish;
                baton->has_result = false;
                baton->has_error = false;

                m_work->data = baton;

                uv_queue_work(
                    event_loop::get().handle(),
                    m_work,
                    [](uv_work_t* req) {
                        task_baton* baton = (task_baton*)req->data;

                        baton->fn(
                            [baton](const Ty& result) {
                                baton->has_error = false;
                                baton->has_result = true;
                                baton->result = result;
                            },
                            [baton](const ErrTy& err) {
                                baton->has_error = true;
                                baton->has_result = false;
                                baton->error = err;
                            });
                    },
                    [](uv_work_t* req, int status) {
                        task_baton* baton = (task_baton*)req->data;

                        if(baton->has_error && baton->on_catch) {
                            baton->on_catch(baton->error);
                        } else if(baton->has_result && baton->on_then) {
                            baton->on_then(baton->result);
                        }

                        if(baton->on_finish) {
                            baton->on_finish();
                        }

                        delete baton;
                        delete req;
                    });
            }

        private:
            task_fn _fn;
            then_fn _then;
            error_fn _error;
            finish_fn _finish;
        };
    }    // namespace event
}    // namespace eqemu
