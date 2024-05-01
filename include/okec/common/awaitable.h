///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_AWAITABLE_H_
#define OKEC_AWAITABLE_H_

#include <coroutine>


namespace okec {

class awaitable_promise_base {
public:
    auto initial_suspend() noexcept -> std::suspend_never;
    [[nodiscard]] auto final_suspend() noexcept -> std::suspend_always;

    auto unhandled_exception() -> void;
    auto return_void() -> void;
};


class awaitable {
    using this_type = awaitable;

public:
    struct promise_type : awaitable_promise_base {
        [[nodiscard]] auto get_return_object() noexcept -> this_type {
            return this_type { std::coroutine_handle<promise_type>::from_promise(*this) };
        }
    };

    awaitable(awaitable&& other) noexcept;
    ~awaitable();
    awaitable& operator=(awaitable other) noexcept;

    void resume();

private:
    std::coroutine_handle<promise_type> handle_ = nullptr;

    explicit(true) awaitable(std::coroutine_handle<promise_type> handle) noexcept;
};

} // namespace okec

#endif // OKEC_AWAITABLE_H_