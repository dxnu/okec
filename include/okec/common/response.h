///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_RESPONSE_H_
#define OKEC_RESPONSE_H_

#include <okec/utils/packet_helper.h>
#include <functional>
#include <string>


namespace okec
{

class response {
public:
    using attribute_type         = std::pair<std::string_view, std::string_view>;
    using attributes_type        = std::initializer_list<attribute_type>;
    using value_type             = json;
    using iterator               = json::iterator;
    using unary_predicate_type   = std::function<bool(const value_type&)>;
    using binary_predicate_type  = std::function<bool(const value_type&, const value_type&)>;

public:
    // auto attribute(std::string_view key, std::string_view value) -> void;
    response() = default;
    response(const response& other) noexcept;
    response& operator=(const response& other) noexcept;
    response(response&& other) noexcept;
    response& operator=(response&& other) noexcept;

    auto begin() -> iterator;
    auto end() -> iterator;
    
    auto dump(int indent = -1) -> std::string;

    auto data() const {
        return j_["response"]["items"];
    }

    auto view() -> value_type&;

    auto size() const -> std::size_t;

    auto emplace_back(attributes_type values) -> void;
    // auto emplace_back(attribute_type value) -> void;

    auto find_if(unary_predicate_type pred) -> iterator;
    auto count_if(unary_predicate_type pred) const -> int;
    auto dump_with(unary_predicate_type pred) -> response;

    auto dump_with(attributes_type values) -> response;
    auto dump_with(attribute_type value) -> response;

private:
    auto emplace_back(json item) -> void;

private:
    json j_;
};


} // namespace okec

#endif // OKEC_RESPONSE_H_