///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/common/awaitable.h>
#include <okec/common/simulator.h>
#include <okec/utils/log.h>
#include <utility> // exchange

#include <iostream>


namespace okec {

auto awaitable_promise_base::initial_suspend() noexcept -> std::suspend_always
{
    return {};
}

auto awaitable_promise_base::final_suspend() noexcept -> std::suspend_always
{
    return {};
}

auto awaitable_promise_base::unhandled_exception() -> void
{
}

auto awaitable_promise_base::return_void() -> void
{
}

awaitable::awaitable(awaitable &&other) noexcept
    : handle_{ std::exchange(other.handle_, nullptr) }
{
}

awaitable &awaitable::operator=(awaitable&& other) noexcept
{
    if (handle_) handle_.destroy();

    handle_ = std::exchange(other.handle_, nullptr);
    return *this;
}

awaitable::~awaitable()
{
    if (handle_) {
        std::cout << "awaitable is about to be destroyed\n";
        handle_.destroy();
    }
}

void awaitable::resume()
{
    if (handle_ && !handle_.done())
        handle_.resume();
}

void awaitable::start()
{
    resume();
}

awaitable::awaitable(std::coroutine_handle<promise_type> handle) noexcept
    : handle_{handle}
{
}

response_awaiter::response_awaiter(simulator& sim)
    : sim{ sim }
{
}

auto response_awaiter::await_ready() noexcept -> bool
{
    log::debug("response_awaiter::await_ready()");
    return false;
}

auto response_awaiter::await_suspend(std::coroutine_handle<> handle) noexcept -> void
{
    log::debug("response_awaiter::await_suspend()");
    sim.completion = [this, handle](response resp) {
        this->r = std::move(resp);
        handle.resume();
    };
}

auto response_awaiter::await_resume() noexcept -> response
{
    log::debug("response_awaiter::await_resume()");
    return std::move(this->r);
}

} // namespace okec
