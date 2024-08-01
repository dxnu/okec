///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_DECISION_ENGINE_H_
#define OKEC_DECISION_ENGINE_H_

#include <okec/common/task.h>
#include <okec/common/resource.h>
#include <okec/utils/packet_helper.h>


namespace okec
{

class base_station;
class base_station_container;
class client_device;
class edge_device;
class cloud_server;


class device_cache
{
public:
    using attribute_type   = std::pair<std::string_view, std::string_view>;
    using attributes_type  = std::initializer_list<attribute_type>;
    using value_type       = json;
    using iterator         = json::iterator;
    using const_iterator   = json::const_iterator;
    using unary_predicate_type  = std::function<bool(const value_type&)>;
    using binary_predicate_type = std::function<bool(const value_type&, const value_type&)>;

public:

    auto begin() -> iterator;
    auto end() -> iterator;

    auto cbegin() const -> json::const_iterator;
    auto cend() const -> json::const_iterator;

    auto dump(int indent = -1) const -> std::string;

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
    : public std::enable_shared_from_this<decision_engine>
{
protected:
    std::shared_ptr<base_station> m_decision_device;

    using result_t = json;

    template <class Derived>
    auto shared_from_base() -> std::shared_ptr<Derived> {
        return std::static_pointer_cast<Derived>(this->shared_from_this());
    }

    auto resource_changed(edge_device* es, ns3::Ipv4Address remote_ip, uint16_t remote_port) -> void;
    auto conflict(edge_device* es, const task_element& item, ns3::Ipv4Address remote_ip, uint16_t remote_port) -> void;

public:
    virtual ~decision_engine() {}

    auto calculate_distance(const ns3::Vector& pos) -> double;
    auto calculate_distance(double x, double y, double z) -> double;

    auto initialize_device(base_station_container* bs_container, cloud_server* cs) -> void;
    auto initialize_device(base_station_container* bs_container) -> void;
    
    virtual auto make_decision(const task_element& header) -> result_t = 0;
    
    virtual auto local_test(const task_element& header, client_device* client) -> bool = 0;

    virtual auto send(task_element t, std::shared_ptr<client_device> client) -> bool = 0;

    virtual auto initialize() -> void = 0;

    virtual auto handle_next() -> void = 0;

    auto get_decision_device() const -> std::shared_ptr<base_station>;

    auto cache() -> device_cache&;

private:
    device_cache m_device_cache;
    std::pair<ns3::Ipv4Address, uint16_t> m_cs_address;
    std::tuple<ns3::Ipv4Address, uint16_t, ns3::Vector> m_cs_info;
};



#define TO_STR(e) e.template get<std::string>()
#define TO_INT(e) std::stoi(TO_STR(e))
#define TO_DOUBLE(e) std::stod(TO_STR(e))


} // namespace okec

#endif // OKEC_DECISION_ENGINE_H_