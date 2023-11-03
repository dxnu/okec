#ifndef OKEC_MESSAGE_H_
#define OKEC_MESSAGE_H_

#include "task.h"
#include "response.h"
#include "nlohmann/json.hpp"
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

    auto to_task() -> Ptr<task>;

    auto to_response() -> Ptr<response>;

    auto to_packet() -> Ptr<Packet>;

    static auto from_packet(Ptr<Packet> packet) -> message;

    auto content(Ptr<task> t) -> void;

    auto content(Ptr<response> r) -> void;

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


inline constexpr std::string_view message_offloading_task { "offloading-task" };
inline constexpr std::string_view message_response { "response" };
inline constexpr std::string_view message_handling { "handling" };
inline constexpr std::string_view message_dispatching { "dispatching" };
inline constexpr std::string_view message_dispatching_failure { "dispatching-failure" };
inline constexpr std::string_view message_dispatching_success { "dispatching-success" };
inline constexpr std::string_view message_decision { "decision" };
// inline constexpr std::string_view 

} // namespace okec

#endif // OKEC_MESSAGE_H_