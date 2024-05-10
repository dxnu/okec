#include <okec/common/simulator.h>
#include <okec/common/response.h>
#include <okec/config/config.h>
#include <okec/utils/log.h>



namespace okec
{

simulator::simulator(ns3::Time time)
    : stop_time_{std::move(time)}
{
    // log::debug("C++ version: {}", __cplusplus);
    ns3::Time::SetResolution(ns3::Time::NS);
}

simulator::~simulator()
{
    ns3::Simulator::Destroy();
}

auto simulator::run() -> void
{
    ns3::Simulator::Stop(stop_time_);
    ns3::Simulator::Run();
}

auto simulator::stop_time(ns3::Time time) -> void
{
    stop_time_ = std::move(time);
}

auto simulator::stop_time() const -> ns3::Time
{
    return stop_time_;
}

auto simulator::enable_visualizer() -> void
{
    ns3::GlobalValue::Bind("SimulatorImplementationType", ns3::StringValue("ns3::VisualSimulatorImpl"));
}

auto simulator::submit(const std::string &ip, std::function<void(response &&)> fn) -> void
{
    completion_[ip] = fn;
}

auto simulator::complete(const std::string& ip, response&& r) -> void
{
    if (auto it = completion_.find(ip);
        it != completion_.end()) {
        auto fn = it->second;
        completion_.erase(it);
        fn(std::move(r));
    }
}

auto simulator::is_valid(const std::string &ip) -> bool
{
    if (auto it = completion_.find(ip);
        it != completion_.end()) {
        return true;
    }

    return false;
}

auto simulator::hold_coro(awaitable a) -> void
{
    coros_.emplace_back(std::move(a));
    // coros_[coros_.size() - 1].start();
}


} // namespace okec