#ifndef NS3_RESOURCE_H_
#define NS3_RESOURCE_H_

#include "ns3/core-module.h"
#include "ns3/node-container.h"


using namespace ns3;

namespace ns3
{


class Resource : public Object
{
    using price_type = long long;

public:
    static TypeId GetTypeId();

    Resource();

    /**
     * \brief Install resource on node.
    */
    void Install(Ptr<Node> node);

    /**
     * \brief Install resource on nodes. 
    */
    // void Install(const NodeContainer &nodes);

    /**
     * \brief Get the resource provider.
    */
    std::string GetProvider() const;

    /**
     * \brief Set the resource provider.
    */
    void SetProvider(std::string_view provider);

    /**
     * \brief Get the equipment memory.
    */
    double GetMemory() const;

    /**
     * \brief Set the equipment memory.
    */
    void SetMemory(double memory);

    /**
     * \brief Get the equipment CPU.
    */
    double GetCpu() const;

    /**
     * \brief Set the equipment CPU.
    */
    void SetCpu(double cpu);

    static void PrintAllResources(const NodeContainer& nodeContainer);

    price_type GetPrice() const {
        return m_price ;
    }

    void SetPrice(price_type price ) {
        m_price  = price ;
    }

    void Build(std::string_view provider, double memory, double cpu, price_type price);

private:
    std::string m_provider;       // 供应商
    std::string m_resourceId;     // 资源ID
    TracedValue<double> m_memory; // 内存
    TracedValue<double> m_cpu;    // CPU
    price_type m_price ;          // 标价（暂时用long表示）
};


class ResourceContainer
{
public:
    ResourceContainer(std::size_t n);

    Ptr<Resource> operator[](std::size_t index);
    Ptr<Resource> operator()(std::size_t index);

    Ptr<Resource> Get(std::size_t index);

    std::size_t GetSize() const;

    auto begin();

    auto end();

private:
    std::vector<Ptr<Resource>> m_resources;
};


} // namespace ns3

#endif // NS3_RESOURCE_H_