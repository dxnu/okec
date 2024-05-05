#ifndef OKEC_SIMULATOR_H_
#define OKEC_SIMULATOR_H_

#include <okec/common/awaitable.h>
#include <functional>
#include <ns3/core-module.h>

namespace okec {

class response;

class simulator {
public:
    std::function<void(response)> completion;
    awaitable coro;

    simulator(ns3::Time time = ns3::Seconds(300));
    ~simulator();

    auto run() -> void;

    auto stop_time(ns3::Time time) -> void;
    auto stop_time() const -> ns3::Time;

    void complete(response r);

    explicit operator bool() const;

private:
    ns3::Time stop_time_;
};


} // namespace okec

#endif // OKEC_SIMULATOR_H_