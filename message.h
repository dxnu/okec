#ifndef OKEC_MESSAGE_H_
#define OKEC_MESSAGE_H_

#include "task.h"
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

    auto dump() -> std::string;

    auto type(std::string_view sv) -> void;
    auto type() -> std::string;

    static auto to_task(Ptr<Packet> packet) -> Ptr<task>;

    auto to_packet() -> Ptr<Packet>;

    auto content(task& t) -> void;

    template <typename Type>
    auto content() -> Type {
        Type result{};
        if (j_.contains("content"))
            result = j_["content"].get<Type>();
        
        return result;
    }

    operator json&() { return j_; }

private:
    json j_;
};


inline constexpr std::string_view message_offloading_task { "offloading-task" };
inline constexpr std::string_view message_response { "response" };
// inline constexpr std::string_view 

} // namespace okec

#endif // OKEC_MESSAGE_H_