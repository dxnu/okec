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
#include <utility> // exchange


namespace okec {

auto awaitable_promise_base::initial_suspend() noexcept -> std::suspend_never
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

awaitable::~awaitable()
{
    if (handle_)
        handle_.destroy();
}

awaitable &awaitable::operator=(awaitable other) noexcept
{
    
    std::swap(handle_, other.handle_);
    return *this;
}

void awaitable::resume()
{
    if (!handle_.done())
        handle_.resume();
}

awaitable::awaitable(std::coroutine_handle<promise_type> handle) noexcept
    : handle_{ handle }
{
}

} // namespace okec
