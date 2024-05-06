#ifndef OKEC_SIMULATOR_H_
#define OKEC_SIMULATOR_H_

#include <okec/common/awaitable.h>
#include <functional>
#include <ns3/core-module.h>

namespace okec {

class response;

class simulator {
public:
    simulator(ns3::Time time = ns3::Seconds(300));
    ~simulator();

    auto run() -> void;

    auto stop_time(ns3::Time time) -> void;
    auto stop_time() const -> ns3::Time;

    auto submit(const std::string& ip, std::function<void(response&&)> fn) -> void;

    auto complete(const std::string& ip, response&& r) -> void;

    auto is_valid(const std::string& ip) -> bool;

    auto hold_coro(awaitable a) -> void;

private:
    ns3::Time stop_time_;
    std::vector<awaitable> coros_;
    std::unordered_map<std::string, std::function<void(response&&)>> completion_;
};


} // namespace okec

#endif // OKEC_SIMULATOR_H_