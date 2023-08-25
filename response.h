#ifndef OKEC_RESPONSE_H
#define OKEC_RESPONSE_H


#include "packet_helper.h"
#include <string>


namespace okec
{


class response : public ns3::SimpleRefCount<response>
{
public:
    response();

    // 处理设备 [device_type, device_address]
    auto handling_device(const std::string &type, const std::string &address) -> void;
    auto handling_device() -> std::pair<const std::string, const std::string>;

    // 任务ID
    auto task_id() const -> std::string;
    auto task_id(std::string id) -> void;

    auto group() const -> std::string;
    auto group(std::string g) -> void;

    // 响应是否为空
    auto empty() -> bool;

    // 转换为Packet
    auto to_packet() const -> Ptr<Packet>;

    auto to_string() const -> std::string;

    // 响应大小
    auto size() -> std::size_t;

private:
    json m_response;
};

auto make_response() -> Ptr<response>;


} // namespace okec

#endif // OKEC_RESPONSE_H