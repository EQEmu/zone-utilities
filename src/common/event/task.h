#pragma once

#include <cstring>
#include <functional>
#include <optional>
#include <string>

#include "event_loop.h"

namespace eqemu {
    namespace event {
        class task_error {
        public:
            task_error() : _err(""), _err_code(0) {}
            task_error(const std::string& err, int err_code) : _err(err), _err_code(err_code) {}

            operator bool() const { return _err_code != 0 || false != _err.empty(); }

        private:
            std::string _err;
            int _err_code;
        };

        template <typename Ty>
        class task {
        public:
            typedef std::function<void(const std::optional<Ty>&)> success_fn;
            typedef std::function<void(const task_error&)> failure_fn;
            typedef std::function<void()> finally_fn;
            typedef std::function<void(success_fn, failure_fn)> task_fn;

        private:
            struct task_baton {
                task_fn fn;
                success_fn on_success;
                failure_fn on_failure;
                finally_fn on_finally;
                task_error error;
                std::optional<Ty> result;
            };

        public:
            task(task_fn fn) { _fn = fn; }

            task& success(success_fn fn) {
                _success_fn = fn;
                return *this;
            }

            task& failure(failure_fn fn) {
                _failure_fn = fn;
                return *this;
            }

            task& finally(finally_fn fn) {
                _finally_fn = fn;
                return *this;
            }

            void run(event_loop& loop) {
                uv_work_t* work = new uv_work_t;
                memset(work, 0, sizeof(uv_work_t));

                auto baton = new task_baton();
                baton->fn = _fn;
                baton->on_success = _success_fn;
                baton->on_failure = _failure_fn;
                baton->on_finally = _finally_fn;
                work->data = baton;

                uv_queue_work(
                    loop.handle(),
                    work,
                    [](uv_work_t* req) {
                        auto baton = (task_baton*)req->data;

                        baton->fn([baton](const std::optional<Ty>& result) { baton->result = result; },
                                  [baton](const task_error& err) { baton->error = err; });
                    },
                    [](uv_work_t* req, int status) {
                        auto baton = (task_baton*)req->data;

                        if(baton->error && baton->on_failure) {
                            baton->on_failure(baton->error);
                        } else if(baton->on_success) {
                            baton->on_success(baton->result);
                        }

                        if(baton->on_finally) {
                            baton->on_finally();
                        }

                        delete baton;
                        delete req;
                    });
            }

        private:
            task_fn _fn;
            success_fn _success_fn;
            failure_fn _failure_fn;
            finally_fn _finally_fn;
        };

        // struct task_void_error {};
        // struct task_void_type {};
        //
        // template <typename Ty = task_void_type, typename ErrTy = task_void_error>
        // class task {
        // public:
        //    typedef std::function<void(const Ty&)> then_fn;
        //    typedef std::function<void(const ErrTy&)> error_fn;
        //    typedef std::function<void()> finish_fn;
        //    typedef std::function<void(then_fn, error_fn)> task_fn;
        //
        //    struct task_baton {
        //        task_fn fn;
        //        then_fn on_then;
        //        error_fn on_catch;
        //        finish_fn on_finish;
        //        bool has_result;
        //        Ty result;
        //        bool has_error;
        //        ErrTy error;
        //    };
        //
        //    task(task_fn fn) { _fn = fn; }
        //
        //    ~task() {}
        //
        //    task& then(then_fn fn) {
        //        _then = fn;
        //        return *this;
        //    }
        //
        //    task& error(error_fn fn) {
        //        _error = fn;
        //        return *this;
        //    }
        //
        //    task& finally(finish_fn fn) {
        //        _finish = fn;
        //        return *this;
        //    }
        //
        //    void run() {
        //        uv_work_t* m_work = new uv_work_t;
        //        memset(m_work, 0, sizeof(uv_work_t));
        //        task_baton* baton = new task_baton();
        //        baton->fn = _fn;
        //        baton->on_then = _then;
        //        baton->on_catch = _error;
        //        baton->on_finish = _finish;
        //        baton->has_result = false;
        //        baton->has_error = false;
        //
        //        m_work->data = baton;
        //
        //        uv_queue_work(
        //            event_loop::get().handle(),
        //            m_work,
        //            [](uv_work_t* req) {
        //                task_baton* baton = (task_baton*)req->data;
        //
        //                baton->fn(
        //                    [baton](const Ty& result) {
        //                        baton->has_error = false;
        //                        baton->has_result = true;
        //                        baton->result = result;
        //                    },
        //                    [baton](const ErrTy& err) {
        //                        baton->has_error = true;
        //                        baton->has_result = false;
        //                        baton->error = err;
        //                    });
        //            },
        //            [](uv_work_t* req, int status) {
        //                task_baton* baton = (task_baton*)req->data;
        //
        //                if(baton->has_error && baton->on_catch) {
        //                    baton->on_catch(baton->error);
        //                } else if(baton->has_result && baton->on_then) {
        //                    baton->on_then(baton->result);
        //                }
        //
        //                if(baton->on_finish) {
        //                    baton->on_finish();
        //                }
        //
        //                delete baton;
        //                delete req;
        //            });
        //    }
        //
        // private:
        //    task_fn _fn;
        //    then_fn _then;
        //    error_fn _error;
        //    finish_fn _finish;
        //};
    }    // namespace event
}    // namespace eqemu
