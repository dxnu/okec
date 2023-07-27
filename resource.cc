#include "resource.h"
#include "format_helper.hpp"
#include <random>


#define CHECK_INDEX(index) \
if (index > size()) throw std::out_of_range{"index out of range"}


namespace okec
{

NS_OBJECT_ENSURE_REGISTERED(resource);

resource::resource()
    : m_cpu_cycles{}
    , m_memory{}
    , m_id{}
    , m_price{}
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

auto resource::cpu_cycles() const -> int
{
    return m_cpu_cycles;
}

auto resource::cpu_cycles(int cycles) -> void
{
    m_cpu_cycles = cycles;
}

auto resource::id() const -> std::string
{
    return m_id;
}

auto resource::id(std::string id) -> void
{
    m_id = std::move(id);
}

auto resource::memory() const -> int
{
    return m_memory;
}

auto resource::memory(int mem) -> void
{
    m_memory = mem;
}

auto resource::price() const -> price_type
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

resource_container::resource_container(std::size_t n)
{
    m_resources.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
        m_resources.emplace_back(make_resource());
}

auto resource_container::operator[](std::size_t index) -> Ptr<resource>
{
    return get(index);
}

auto resource_container::operator()(std::size_t index) -> Ptr<resource>
{
    return get(index);
}

auto resource_container::get(std::size_t index) -> Ptr<resource>
{
    CHECK_INDEX(index);
    return m_resources[index];
}

auto resource_container::random_initialization() -> void
{
    // 生成 ID
    int id_len = this->size();
    int width = 8;
    std::vector<int> id_vec(id_len);
    std::vector<std::string> id_data(id_len);
    std::iota(id_vec.begin(), id_vec.end(), 0);
    std::transform(id_vec.begin(), id_vec.end(), id_data.begin(), [&width](int n) {
        auto sn = std::to_string(n);
        return std::string(width - std::min(width, (int)sn.length()), '0') + sn; 
    });


    using rng = std::default_random_engine;
    static rng dre{ (rng::result_type)time(0) };

    // 生成需要的 cpu_cycles
    std::uniform_int_distribution<uint> cpu_cycles_uid(0, 5000);

    // 生成需要的 memory
    std::uniform_int_distribution<uint> memory_uid(0, 4000);

    // 生成 price
    std::uniform_int_distribution<uint> price_uid(0, 10000);

    for (std::size_t i = 0; i < size(); ++i) {
        m_resources[i]->id(id_data[i]);
        m_resources[i]->cpu_cycles(cpu_cycles_uid(dre));
        m_resources[i]->memory(memory_uid(dre));
        m_resources[i]->price(price_uid(dre));
    }
}

std::size_t resource_container::size() const
{
    return m_resources.size();
}

auto resource_container::print(std::string title) -> void
{
    if (!title.empty())
        fmt::print("{}\n", title);

    fmt::print("{:rs}\n", *this);
}

} // namespace okec