#ifndef OKEC_RESPONSE_H
#define OKEC_RESPONSE_H


#include "packet_helper.h"

#include <functional>
#include <string>


namespace okec
{

class response {
public:
    using attribute_type   = std::pair<std::string_view, std::string_view>;
    using attributes_type  = std::initializer_list<attribute_type>;
    using value_type       = json;
    using iterator         = json::iterator;
    using unary_predicate_type   = std::function<bool(const value_type&)>;
    using binary_predicate_type   = std::function<bool(const value_type&, const value_type&)>;

public:
    // auto attribute(std::string_view key, std::string_view value) -> void;
    response() = default;
    response(const response& other) noexcept;
    response& operator=(const response& other) noexcept;
    response(response&& other) noexcept;
    response& operator=(response&& other) noexcept;

    auto begin() -> iterator;
    auto end() -> iterator;
    
    auto dump() -> std::string;

    auto data() const {
        return j_["response"]["items"];
    }

    auto view() -> value_type&;

    auto size() const -> std::size_t;

    auto emplace_back(attributes_type values) -> void;
    // auto emplace_back(attribute_type value) -> void;

    auto set_if(attributes_type values, auto f) -> void
    {
        for (auto& item : j_["response"]["items"])
        {
            bool cond{true};
            for (auto [key, value] : values)
            {
                if (item[key] != value)
                    cond = false;
            }

            if (cond)
            {
                f(item);
                break;
            }
        }
    }

    auto find_if(attributes_type values) const -> bool;
    auto find_if(attribute_type value) const -> bool;

    auto find_if(unary_predicate_type pred) -> iterator;

    auto count_if(attributes_type values) const -> int;
    auto count_if(attribute_type value) const -> int;

    auto dump_with(attributes_type values) -> response;
    auto dump_with(attribute_type value) -> response;

private:
    auto emplace_back(json item) -> void;

private:
    json j_;
};


} // namespace okec

#endif // OKEC_RESPONSE_H