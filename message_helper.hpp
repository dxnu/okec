#ifndef OKEC_MESSAGE_HELPER_
#define OKEC_MESSAGE_HELPER_

#include "message.h"
#include "message_handler.hpp"


namespace okec
{


static inline auto get_message_type(Ptr<Packet> packet) {
    std::string result{};
    json j = to_json(packet);
    if (!j.is_null() && j.contains("msgtype")) {
        result = j["msgtype"].get<std::string>();
    }

    return result;
}

template <typename CallbackType>
static inline decltype(auto) get_message_handler() {
	return utils::singleton<message_handler<CallbackType>>::instance();
}

template <typename CallbackType>
static inline auto push_message_handler(std::initializer_list<std::pair<std::string, CallbackType>> values) {
	for (auto&& [key, value] : values) {
		get_message_handler<CallbackType>().add_handler(
			std::forward<decltype(key)>(key), std::forward<decltype(value)>(value));
	}
}

template <typename CallbackType>
static inline auto push_message_handler(std::pair<std::string, CallbackType> value) {
	push_message_handler<CallbackType>({ value });
}


} // namespace okec

#endif // OKEC_MESSAGE_HELPER_