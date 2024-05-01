///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_MESSAGE_HANDLER_H_
#define OKEC_MESSAGE_HANDLER_H_

#include <okec/utils/delegate.hpp>
#include <functional>
#include <string>


namespace okec
{

template <typename CallbackType = std::function<void()>>
class message_handler {
	using delegate_type = utils::delegate<std::string_view, CallbackType>;

public:
	auto add_handler(std::string_view msg_type, CallbackType callback) -> void {
		delegate_.insert(msg_type, callback);
	}

	template <typename... Args>
	auto dispatch(const std::string& msg_type, Args... args) -> bool {
		typename delegate_type::value_type::const_iterator iter;
		bool ret = delegate_.find(msg_type, iter);
		if (ret) {
			iter->second(args...);
		}

		return ret;
	}

private:
	delegate_type delegate_;
};


} // namespace okec

#endif // OKEC_MESSAGE_HANDLER_H_