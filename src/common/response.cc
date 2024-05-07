///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/common/response.h>
#include <okec/utils/log.h>

namespace okec
{

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
    : j_{ std::move(other.j_) }
{
}

response& response::operator=(response&& other) noexcept
{
    j_ = std::move(other.j_);
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

auto response::dump(int indent) -> std::string
{
    return j_.dump(indent);
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

auto response::find_if(unary_predicate_type pred) -> iterator
{
    auto& items = this->view();
    return std::find_if(items.begin(), items.end(), pred);
}

auto response::count_if(unary_predicate_type pred) const -> int
{
    const auto& items = j_["response"]["items"];
    return std::count_if(items.begin(), items.end(), pred);
}

auto response::dump_with(unary_predicate_type pred) -> response
{
    response result;
    auto& items = this->view();
    for (auto it = items.begin(); it != items.end();) {
        if (pred(*it)) {
            result.emplace_back(std::move(*it));
            it = items.erase(it);
        } else {
            it++;
        }
    }
    return result;
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