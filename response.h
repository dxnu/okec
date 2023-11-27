#ifndef OKEC_RESPONSE_H
#define OKEC_RESPONSE_H


#include "packet_helper.h"
#include <string>

namespace okec
{




// class response : public ns3::SimpleRefCount<response>
// {
// public:
//     response();

//     // 处理设备 [device_type, device_address]
//     auto handling_device(const std::string &type, const std::string &address) -> void;
//     auto handling_device() -> std::pair<const std::string, const std::string>;

//     // 任务ID
//     auto task_id() const -> std::string;
//     auto task_id(std::string id) -> void;

//     auto group() const -> std::string;
//     auto group(std::string g) -> void;

//     // 响应是否为空
//     auto empty() -> bool;

//     // 转换为Packet
//     auto to_packet() const -> Ptr<Packet>;

//     auto to_string() const -> std::string;

//     // 响应大小
//     auto size() -> std::size_t;

// private:
//     json m_response;
// };

// auto make_response() -> Ptr<response>;

class response {
    using attribute_t      = std::pair<std::string_view, std::string_view>;
    using attributes_t     = std::initializer_list<attribute_t>;

public:
    // auto attribute(std::string_view key, std::string_view value) -> void;
    
    auto dump() -> std::string;

    auto data() const {
        return j_["response"]["items"];
    }

    auto size() const -> std::size_t;

    auto emplace_back(attributes_t values) -> void;
    // auto emplace_back(attribute_t value) -> void;

    auto set_if(attributes_t values, auto f) -> void
    {
        for (auto& item : j_["response"]["items"])
        {
            bool cond{true};
            for (auto [key, value] : values)
            {
                if (item[key] != value)
                    cond = false;
            }

            if (cond)
            {
                f(item);
                break;
            }
        }
    }

    auto find_if(attributes_t values) const -> bool;
    auto find_if(attribute_t value) const -> bool;

    auto count_if(attributes_t values) const -> int;
    auto count_if(attribute_t value) const -> int;

    auto dump_with(attributes_t values) -> response;
    auto dump_with(attribute_t value) -> response;

private:
    auto emplace_back(json item) -> void;

private:
    json j_;
};


} // namespace okec

#endif // OKEC_RESPONSE_H