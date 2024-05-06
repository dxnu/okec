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

#include <okec/common/response.h>
#include <coroutine>


namespace okec {

class client_device;
class simulator;


class awaitable_promise_base {
public:
    auto initial_suspend() noexcept -> std::suspend_never;
    [[nodiscard]] auto final_suspend() noexcept -> std::suspend_always;

    auto unhandled_exception() -> void;
    auto return_void() -> void;
};


class awaitable {
public:
    struct promise_type : awaitable_promise_base {
        [[nodiscard]] auto get_return_object() noexcept -> awaitable {
            return awaitable { std::coroutine_handle<promise_type>::from_promise(*this) };
        }
    };

    awaitable() = default;
    awaitable(awaitable&& other) noexcept;
    awaitable& operator=(awaitable&& other) noexcept;
    ~awaitable();

    void resume();

    // void start();

private:
    std::coroutine_handle<promise_type> handle_ = nullptr;

    explicit(true) awaitable(std::coroutine_handle<promise_type> handle) noexcept;
    awaitable(const awaitable&) = delete;
    awaitable& operator=(const awaitable&) = delete;
};

class response_awaiter {
public:
    response_awaiter(simulator& sim, std::string client_address);
    auto await_ready() noexcept -> bool;
    auto await_suspend(std::coroutine_handle<> handle) noexcept -> void;
    [[nodiscard]] auto await_resume() noexcept -> response;

private:
    simulator& sim;
    std::string client_address;
    response r;
};

auto co_spawn(okec::simulator& ctx, okec::awaitable a) -> void;

} // namespace okec

#endif // OKEC_AWAITABLE_H_