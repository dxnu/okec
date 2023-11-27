#ifndef OKEC_DECISION_ENGINE_
#define OKEC_DECISION_ENGINE_

#include "task.h"
#include "resource.h"
#include "packet_helper.h"


namespace okec
{

class base_station;
class base_station_container;
class cloud_server;


class device_cache_t
{
    using attribute_t      = std::pair<std::string_view, std::string_view>;
    using attributes_t     = std::initializer_list<attribute_t>;
public:
    using value_type       = attribute_t;
    using values_type      = attributes_t;

public:

    auto dump() -> std::string;

    auto dump_first() -> std::string;

    auto data() const -> json;

    auto size() const -> std::size_t;

    auto empty() const -> bool;

    auto emplace_back(attributes_t values) -> void;

    auto set_if(attributes_t values, auto f) -> void
    {
        for (auto& item : this->cache["device_cache"]["items"])
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

    auto find_if(attributes_t values) -> device_cache_t;
    auto find_if(attribute_t value) -> device_cache_t;

private:
    auto emplace_back(json item) -> void;

private:
    json cache;
};


class decision_engine
{
protected:
    using result_t = std::tuple<std::string, uint16_t, double>;

    // auto query_information() -> void;

public:
    virtual ~decision_engine() {}

    auto calculate_distance(const Vector& pos) -> double;

    auto initialize_device(base_station_container* bs_container, cloud_server* cs) -> void;
    
    virtual auto make_decision(const task_element& header) -> result_t = 0;

    auto device_cache() const -> json;

    auto find_device_cache(device_cache_t::values_type values) -> json;
    auto find_device_cache(device_cache_t::value_type value) -> json;

private:
    device_cache_t m_device_cache;
    std::pair<ns3::Ipv4Address, uint16_t> m_cs_address;
    std::tuple<ns3::Ipv4Address, uint16_t, Vector> m_cs_info;
    std::shared_ptr<base_station> m_bs_socket;
};



#define TO_INT(e) std::stoi(e.template get<std::string>())
#define TO_DOUBLE(e) std::stod(e.template get<std::string>())

} // namespace okec

#endif // OKEC_DECISION_ENGINE_