#include <okec/common/simulator.h>
#include <okec/common/response.h>
#include <okec/config/config.h>
#include <okec/utils/log.h>


namespace okec
{

simulator::simulator(ns3::Time time)
    : stop_time_{std::move(time)}
{
    log::debug("C++ version: {}", __cplusplus);
    ns3::Time::SetResolution(ns3::Time::NS);
    // ns3::LogComponentEnable("udp_application", static_cast<ns3::LogLevel>(ns3::LOG_LEVEL_INFO | ns3::LOG_PREFIX_TIME));
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

void simulator::complete(response r)
{
    completion(std::move(r));
}

simulator::operator bool() const
{
    return completion ? true : false;
}

} // namespace okec