#ifndef OKEC_RESOURCE_H
#define OKEC_RESOURCE_H

#include "packet_helper.h"
#include "ns3/core-module.h"
#include "ns3/node-container.h"

using namespace ns3;


namespace okec
{


class resource : public ns3::Object
{
    using price_type = int;

public:

    static auto GetTypeId() -> TypeId;

    auto install(Ptr<Node> node) -> void;

    ///////////////////////////////////
    resource() = default;
    resource(json item) noexcept;

    auto attribute(std::string_view key, std::string_view value) -> void;

    auto reset_value(std::string_view key, std::string_view value) -> std::string;

    auto get_value(std::string_view key) const -> std::string;
    
    auto dump() -> std::string;

    auto begin() const {
        return this->empty() ? json::const_iterator() : j_["resource"].begin();
    }

    auto end() const {
        return this->empty() ? json::const_iterator() : j_["resource"].end();
    }

    auto empty() const -> bool;

    auto j_data() const -> json;

    static auto from_msg_packet(Ptr<Packet> packet) -> resource;

private:
    json j_;
};


// factory function
auto make_resource() -> Ptr<resource>;


class resource_container
{
public:
    resource_container(std::size_t n);

    auto operator[](std::size_t index) -> Ptr<resource>;
    auto operator()(std::size_t index) -> Ptr<resource>;

    auto get(std::size_t index) -> Ptr<resource>;

    auto initialize(std::function<void(Ptr<resource>)> fn) -> void;

    auto size() const -> std::size_t;

    auto begin() {
        return m_resources.begin();
    }

    auto end() {
        return m_resources.end();
    }

    auto print(std::string title = "Resource Info" ) -> void;

private:
    std::vector<Ptr<resource>> m_resources;
};

} // namespace okec

#endif // OKEC_RESOURCE_H