#include "response.h"

namespace okec
{

// response::response()
// {
// }

// auto response::handling_device(const std::string& type, const std::string& address) -> void
// {
//     m_response["device_type"] = type;
//     m_response["device_address"] = address;
// }

// auto response::handling_device() -> std::pair<const std::string, const std::string>
// {
//     return std::make_pair(m_response["device_type"], m_response["device_address"]);
// }

// auto response::task_id() const -> std::string
// {
//     return m_response["task_id"];
// }

// auto response::task_id(std::string id) -> void
// {
//     m_response["task_id"] = std::move(id);
// }

// auto response::group() const -> std::string
// {
//     return m_response["group"];
// }

// auto response::group(std::string g) -> void
// {
//     m_response["group"] = std::move(g);
// }

// auto response::empty() -> bool
// {
//     return m_response.is_null();
// }

// auto response::to_packet() const -> Ptr<Packet>
// {
//     return packet_helper::make_packet(m_response.dump());
// }

// auto response::to_string() const -> std::string
// {
//     return m_response.dump();
// }

// auto response::size() -> std::size_t
// {
//     return m_response.size();
// }

// auto make_response() -> Ptr<response>
// {
//     return ns3::Create<response>();
// }

// auto response::attribute(std::string_view key, std::string_view value) -> void
// {
//     j_["response"]["items"][key] = value;
// }

response::response(const response& other) noexcept
{
    if (this != &other) {
        this->j_ = other.j_;
    }
}

response& response::operator=(const response& other) noexcept
{
    if (this != &other) {
        this->j_ = other.j_;
    }

    return *this;
}

response::response(response&& other) noexcept
    : j_{ std::exchange(other.j_, {}) }
{
}

response& response::operator=(response&& other) noexcept
{
    j_ = std::exchange(other.j_, {});
    return *this;
}

auto response::begin() -> iterator
{
    return this->view().begin();
}

auto response::end() -> iterator
{
    return this->view().end();
}

auto response::dump() -> std::string
{
    return j_.dump();
}

auto response::view() -> value_type&
{
    return j_["response"]["items"];
}

auto response::size() const -> std::size_t
{
    return data().size();
}

auto response::emplace_back(attributes_type values) -> void
{
    json item;
    for (auto [key, value] : values) {
        item[key] = value;
    }

    // j_["response"]["items"].emplace_back(std::move(item));
    this->emplace_back(std::move(item));
}

// auto response::emplace_back(attribute_type value) -> void
// {
//     json item;
//     item[value.first] = value.second;
    
// }

// auto response::get_value(std::string_view key) -> std::string
// {
//     std::string result{};
//     if (j_.contains(key))
//         result = j_[key].get<std::string>();
    
//     return result;
// }

auto response::find_if(attributes_type values) const -> bool
{
    for (const auto& item : j_["response"]["items"])
    {
        bool cond = true;
        for (auto [key, value] : values)
        {
            if (item[key] != value) {
                cond = false;
                break;
            }
        }
        if (cond)
            return true;
    }

    return false;
}

auto response::find_if(attribute_type value) const -> bool
{
    return find_if({value});
}

auto response::find_if(unary_predicate_type pred) -> iterator
{
    auto& items = this->view();
    return std::find_if(items.begin(), items.end(), pred);
}

auto response::count_if(attributes_type values) const -> int
{
    int count{};
    for (const auto& item : j_["response"]["items"])
    {
        bool cond = true;
        for (auto [key, value] : values)
        {
            if (item[key] != value) {
                cond = false;
                break;
            }
        }
        if (cond)
            count++;
    }

    return count;
}

auto response::count_if(attribute_type value) const -> int
{
    return count_if({value});
}

auto response::dump_with(attributes_type values) -> response
{
    response res;
    auto& items = j_["response"]["items"];
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        bool cond = true;
        for (auto [key, value] : values)
        {
            if (items[i][key] != value) {
                cond = false;
                break;
            }
        }

        if (cond)
        {
            res.emplace_back(std::move(items[i]));
        }
    }

    // 清除移出的元素
    for (auto iter = items.begin(); iter != items.end();)
    {
        if (iter->is_null())
            iter = items.erase(iter);
        else
            iter++;
    }

    return res;
}

auto response::dump_with(attribute_type value) -> response
{
    return dump_with({value});
}

auto response::emplace_back(json item) -> void
{
    j_["response"]["items"].emplace_back(std::move(item));
}

} // namespace okec