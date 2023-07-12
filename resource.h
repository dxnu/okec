#ifndef OKEC_RESOURCE_H
#define OKEC_RESOURCE_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"


namespace okec
{

using ns3::Node;
using ns3::Ptr;
using ns3::TypeId;



class resource : public ns3::Object
{
    using price_type = int;

public:
    resource();

    static auto GetTypeId() -> TypeId;

    auto install(Ptr<Node> node) -> void;

    // CPU
    auto cpu_cycles() -> int;
    auto cpu_cycles(int cycles) -> void;

    // 内存
    auto memory() -> int;
    auto memory(int mem) -> void;

    // 价格
    auto price() -> price_type;
    auto price(price_type money) -> void;

private:
    int m_cpu_cycles;
    int m_memory;
    price_type m_price;
};


// factory function
auto make_resource() -> Ptr<resource>;

} // namespace okec

#endif // OKEC_RESOURCE_H