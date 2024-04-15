#ifndef OKEC_MESSAGE_H_
#define OKEC_MESSAGE_H_

#include "task.h"
#include "response.h"
#include "resource.h"
// #include "nlohmann/json.hpp"
#include "include/thirdparty/nlohmann/json.hpp"
#include <string_view>


using json = nlohmann::json;


namespace okec
{


class message {
public:
    message() = default;
    message(Ptr<Packet> packet);
    message(std::initializer_list<std::pair<std::string_view, std::string_view>> values);
    message(const message& other);
    message& operator=(message other) noexcept;
    friend void swap(message& lhs, message& rhs) noexcept;

    auto attribute(std::string_view key, std::string_view value) -> void;
    auto get_value(std::string_view key) -> std::string;

    auto dump() -> std::string;

    auto type(std::string_view sv) -> void;
    auto type() -> std::string;

    auto to_packet() -> Ptr<Packet>;

    static auto from_packet(Ptr<Packet> packet) -> message;

    auto content(const task& t) -> void;

    auto content(const task_element& item) -> void;

    auto content(const resource& r) -> void;

    auto get_task_element() -> task_element;

    template <typename Type>
    auto content() -> Type {
        Type result{};
        if (j_.contains("content"))
            result = j_["content"].get<Type>();
        
        return result;
    }

    operator json&() { return j_; }

    auto valid() -> bool;

private:
    json j_;
};


inline constexpr std::string_view message_resource_changed { "resource_changed" };
inline constexpr std::string_view message_response { "response" };
inline constexpr std::string_view message_handling { "handling" };
inline constexpr std::string_view message_dispatching { "dispatching" };
inline constexpr std::string_view message_get_resource_information { "get_resource_information" };
inline constexpr std::string_view message_resource_information { "resource_information" };
inline constexpr std::string_view message_decision { "decision" };
inline constexpr std::string_view message_conflict { "conflict" };
// inline constexpr std::string_view 

} // namespace okec

#endif // OKEC_MESSAGE_H_