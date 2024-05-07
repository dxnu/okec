///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_RESOURCE_H_
#define OKEC_RESOURCE_H_

#include <okec/utils/packet_helper.h>
#include <ns3/core-module.h>
#include <ns3/node-container.h>



namespace okec
{


class resource : public ns3::Object
{
public:
    // [address, key, old_value, new_value]
    using monitor_type = std::function<void(std::string_view, std::string_view, std::string_view, std::string_view)>;

public:

    static auto GetTypeId() -> ns3::TypeId;

    auto install(ns3::Ptr<ns3::Node> node) -> void;

    ///////////////////////////////////
    resource() = default;
    resource(json item) noexcept;

    auto attribute(std::string_view key, std::string_view value) -> void;

    auto reset_value(std::string_view key, std::string_view value) -> std::string;

    auto set_monitor(monitor_type monitor) -> void;

    auto get_value(std::string_view key) const -> std::string;

    auto get_address() -> ns3::Ipv4Address;
    
    auto dump(const int indent = -1) -> std::string;

    auto begin() const {
        return this->empty() ? json::const_iterator() : j_["resource"].begin();
    }

    auto end() const {
        return this->empty() ? json::const_iterator() : j_["resource"].end();
    }

    auto empty() const -> bool;

    auto j_data() const -> json;

    auto set_data(json item) -> bool;

    static auto from_msg_packet(ns3::Ptr<ns3::Packet> packet) -> resource;

private:
    json j_;
    monitor_type monitor_;
    ns3::Ptr<ns3::Node> node_;
};


// factory function
auto make_resource() -> ns3::Ptr<resource>;


class resource_container
{
public:
    resource_container(std::size_t n);

    auto operator[](std::size_t index) -> ns3::Ptr<resource>;
    auto operator()(std::size_t index) -> ns3::Ptr<resource>;

    auto get(std::size_t index) -> ns3::Ptr<resource>;

    auto initialize(std::function<void(ns3::Ptr<resource>)> fn) -> void;

    auto size() const -> std::size_t;

    auto begin() {
        return m_resources.begin();
    }

    auto end() {
        return m_resources.end();
    }

    auto cbegin() const {
        return m_resources.cbegin();
    }

    auto cend() const {
        return m_resources.cend();
    }

    auto print(std::string title = "Resource Info" ) -> void;

    auto trace_resource() -> void;

    auto save_to_file(const std::string& file) -> void;
    auto load_from_file(const std::string& file) -> bool;

    auto set_monitor(resource::monitor_type monitor) -> void;

private:
    std::vector<ns3::Ptr<resource>> m_resources;
};

} // namespace okec

#endif // OKEC_RESOURCE_H_