#include "resource.h"


namespace okec
{

NS_OBJECT_ENSURE_REGISTERED(resource);

resource::resource()
    : m_cpu_cycles{ 0 },
      m_memory{ 0 },
      m_price{ 0 }
{
}

auto resource::GetTypeId() -> TypeId
{
    static TypeId tid = TypeId("okec::resource")
                        .SetParent(Object::GetTypeId())
                        .AddConstructor<resource>();
    return tid;
}

auto resource::install(Ptr<Node> node) -> void
{
    node->AggregateObject(this);
}

auto resource::cpu_cycles() -> int
{
    return m_cpu_cycles;
}

auto resource::cpu_cycles(int cycles) -> void
{
    m_cpu_cycles = cycles;
}

auto resource::memory() -> int
{
    return m_memory;
}

auto resource::memory(int mem) -> void
{
    m_memory = mem;
}

auto resource::price() -> price_type
{
    return m_price;
}

auto resource::price(price_type money) -> void
{
    m_price = money;
}

auto make_resource() -> Ptr<resource>
{
    return ns3::CreateObject<resource>();
}

} // namespace okec