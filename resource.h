#ifndef OKEC_RESOURCE_H
#define OKEC_RESOURCE_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"

using namespace ns3;


namespace okec
{


class resource : public ns3::Object
{
    using price_type = int;

public:
    resource();

    static auto GetTypeId() -> TypeId;

    auto install(Ptr<Node> node) -> void;

    // CPU
    auto cpu_cycles() const -> int;
    auto cpu_cycles(int cycles) -> void;

    auto id() const -> std::string;
    auto id(std::string id) -> void;

    // 内存
    auto memory() const -> int;
    auto memory(int mem) -> void;

    // 价格
    auto price() const -> price_type;
    auto price(price_type money) -> void;

private:
    int m_cpu_cycles;
    int m_memory;
    std::string m_id;
    price_type m_price;
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

    auto random_initialization() -> void;

    auto size() const -> std::size_t;

    auto begin() {
        return m_resources.begin();
    }

    auto end() {
        return m_resources.end();
    }

    auto print(std::string title = {}) -> void;

private:
    std::vector<Ptr<resource>> m_resources;
};

} // namespace okec

#endif // OKEC_RESOURCE_H