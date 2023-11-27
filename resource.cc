#include "resource.h"
#include "format_helper.hpp"
#include <random>


#define CHECK_INDEX(index) \
if (index > size()) throw std::out_of_range{"index out of range"}


namespace okec
{

NS_OBJECT_ENSURE_REGISTERED(resource);


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

resource::resource(json item) noexcept
{
    if (item.contains("/resource"_json_pointer)) {
        j_ = std::move(item);
    }
}

auto resource::attribute(std::string_view key, std::string_view value) -> void
{
    j_["resource"][key] = value;
}

auto resource::reset_value(std::string_view key, std::string_view value) -> std::string
{
    return std::exchange(j_["resource"][key], value);
}

auto resource::get_value(std::string_view key) const -> std::string
{
    std::string result{};
    json::json_pointer j_key{ "/resource/" + std::string(key) };
    if (j_.contains(j_key))
        j_.at(j_key).get_to(result);
    
    return result;
}

auto resource::dump() -> std::string
{
    return j_.dump();
}

auto resource::empty() const -> bool
{
    return !j_.contains("/resource"_json_pointer);
}

auto resource::j_data() const -> json
{
    return j_;
}

auto resource::from_msg_packet(Ptr<Packet> packet) -> resource
{
    json j = packet_helper::to_json(packet);
    if (!j.is_null() && j.contains("/content/resource"_json_pointer))
        return resource(j["content"]);
    
    return resource{};
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
    std::uniform_int_distribution<uint> cpu_cycles_uid(2000, 10000);

    // 生成需要的 memory
    std::uniform_int_distribution<uint> memory_uid(0, 4000);

    // 生成 price
    std::uniform_int_distribution<uint> price_uid(0, 10000);

    for (std::size_t i = 0; i < size(); ++i) {
        m_resources[i]->attribute("id", id_data[i]);
        m_resources[i]->attribute("cpu_cycle", std::to_string(cpu_cycles_uid(dre)));
        m_resources[i]->attribute("memory", std::to_string(memory_uid(dre)));
        m_resources[i]->attribute("price", std::to_string(price_uid(dre)));
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