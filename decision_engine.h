#ifndef OKEC_DECISION_ENGINE_
#define OKEC_DECISION_ENGINE_

#include "task.h"
#include "resource.h"
#include "packet_helper.h"


namespace okec
{

class base_station;
class base_station_container;
class client_device;
class cloud_server;


class device_cache
{
public:
    using attribute_type   = std::pair<std::string_view, std::string_view>;
    using attributes_type  = std::initializer_list<attribute_type>;
    using value_type       = json;
    using iterator         = json::iterator;
    using unary_predicate_type  = std::function<bool(const value_type&)>;
    using binary_predicate_type = std::function<bool(const value_type&, const value_type&)>;

public:

    auto begin() -> iterator;
    auto end() -> iterator;

    auto dump() -> std::string;

    auto data() const -> value_type;

    auto view() -> value_type&;

    auto size() const -> std::size_t;

    auto empty() const -> bool;

    auto emplace_back(attributes_type values) -> void;

    auto find_if(unary_predicate_type pred) -> iterator;

    auto sort(binary_predicate_type comp) -> void;

private:
    auto emplace_back(value_type item) -> void;

private:
    value_type cache;
};


class decision_engine
{
protected:
    using result_t = std::tuple<std::string, uint16_t, double>;

public:
    virtual ~decision_engine() {}

    auto calculate_distance(const Vector& pos) -> double;

    auto initialize_device(base_station_container* bs_container, cloud_server* cs) -> void;
    auto initialize_device(base_station_container* bs_container) -> void;
    
    virtual auto make_decision(const task_element& header) -> result_t = 0;
    
    virtual auto local_test(const task_element& header, client_device* client) -> bool = 0;

    virtual auto send(task_element& t, client_device* client) -> bool = 0;

    auto get_decision_device() const -> std::shared_ptr<base_station>;

    auto cache() -> device_cache&;

private:
    device_cache m_device_cache;
    std::pair<ns3::Ipv4Address, uint16_t> m_cs_address;
    std::tuple<ns3::Ipv4Address, uint16_t, Vector> m_cs_info;

protected:
    std::shared_ptr<base_station> m_decision_device;
};



#define TO_INT(e) std::stoi(e.template get<std::string>())
#define TO_DOUBLE(e) std::stod(e.template get<std::string>())


} // namespace okec

#endif // OKEC_DECISION_ENGINE_