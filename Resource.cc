#include "Resource.h"
#include "EdgeCommunicatorApplication.h"
#include "format_helper.hpp"
#include <algorithm>


// #define TEST_CODE "\033[45m"
// #define END_CODE "\033[0m"
#define CHECK_INDEX(index) \
if (index > GetSize()) throw std::out_of_range{"index out of range"}


namespace ns3
{


NS_OBJECT_ENSURE_REGISTERED(Resource);


TypeId Resource::GetTypeId()
{
    static TypeId tid = TypeId("ns3::Resource")
                        .SetParent(Object::GetTypeId())
                        .SetGroupName("MyEIP")
                        .AddConstructor<Resource>()
                        .AddTraceSource("ResourceMemory",
                            "Traced the resource memory",
                            MakeTraceSourceAccessor(&Resource::m_memory),
                            "ResourceMemoryTracedCallback")
                        .AddTraceSource("ResourceCpu",
                            "Traced the resource memory",
                            MakeTraceSourceAccessor(&Resource::m_cpu),
                            "ResourceCpuTracedCallback");
    return tid;
}

Resource::Resource()
    : m_provider{""},
      m_memory{0},
      m_cpu{0},
      m_price{0}
{
}

void Resource::Install(Ptr<Node> node)
{
    node->AggregateObject(this);
}

std::string Resource::GetProvider() const
{
    return m_provider;
}

void Resource::SetProvider(std::string_view provider)
{
    m_provider = provider;
}

double Resource::GetMemory() const
{
    return m_memory;
}

void Resource::SetMemory(double memory)
{
    m_memory = memory;
}

double Resource::GetCpu() const
{
    return m_cpu;
}

void Resource::SetCpu(double cpu)
{
    m_cpu = cpu;
}

// void Resource::Install(const NodeContainer &nodes)
// {
//     for (auto it = nodes.Begin(); it != nodes.End(); ++it)
//         Install(*it);
// }

void Resource::PrintAllResources(const NodeContainer& nodeContainer)
{
    const int propertyLen = 100;
    auto allResourceInfo = fmt::format("{0:=^{1}}\n{3:<{2}}{4:{2}}{5:>{2}}{6:>{2}}{7:>{2}}\n{0:=^{1}}\n", 
                            "", propertyLen, propertyLen / 5, "node", "resource provider", "memory", "cpu", "price");

    for (uint32_t i = 0; i < nodeContainer.GetN(); ++i) {
        auto node = nodeContainer.Get(i);
        // Node上面没有任何应用，无效Node，再找
        if (node->GetNApplications() < 1)
            continue;

        // 通过Node找到Communicator，也就找到了IP地址与端口。
        // 其实也可以直接通过Node获取到IP地址，但无法得到端口，且最终的通信依旧要使用Communicator，
        // 因此直接使用Communicator来获取地址和端口，以保证接口的一致性。
        Ptr<EdgeCommunicatorApplication> communicator = node->GetApplication(0)->GetObject<EdgeCommunicatorApplication>();
        if (communicator != nullptr) {
            // 只有边缘设备提供商的Node上面才有资源，没有资源则再找
            auto resource = node->GetObject<Resource>();
            if (resource == nullptr)
                continue;

            Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
            auto address =  fmt::format("{:ip}", ipv4->GetAddress(1, 0).GetLocal());
            allResourceInfo += fmt::format("{1:<{0}}{2:<{0}}{3:>{0}}{4:>{0}}{5:>{0}}\n", 
                                propertyLen / 5, address, resource->GetProvider(), 
                                resource->GetMemory(), resource->GetCpu(), resource->GetPrice());
        }
        
    }

    allResourceInfo += fmt::format("{0:=^{1}}\n", "", propertyLen);
    // NS_LOG_INFO(TEST_CODE << allResourceInfo << END_CODE);
    fmt::print(/*fmt::emphasis::blink | */fg(fmt::color::red), allResourceInfo);
}

void Resource::Build(std::string_view provider, double memory, double cpu, price_type price)
{
    m_provider = provider;
    m_memory = memory;
    m_cpu = cpu;
    m_price = price;
}

ResourceContainer::ResourceContainer(std::size_t n)
{
    m_resources.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
        m_resources.emplace_back(CreateObject<Resource>());
}

Ptr<Resource> ResourceContainer::operator[](std::size_t index)
{
    return Get(index);
}

Ptr<Resource> ResourceContainer::operator()(std::size_t index)
{
    return Get(index);
}

Ptr<Resource> ResourceContainer::Get(std::size_t index)
{
    CHECK_INDEX(index);
    return m_resources[index];
}

std::size_t ResourceContainer::GetSize() const
{
    return m_resources.size();
}

auto ResourceContainer::begin()
{
    return m_resources.begin();
}

auto ResourceContainer::end()
{
    return m_resources.end();
}

} // namespace ns3