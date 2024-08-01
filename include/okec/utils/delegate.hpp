///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_DELEGATE_H_
#define OKEC_DELEGATE_H_

#include <map>


namespace okec::utils 
{

template <typename IdentifierType, typename CallbackType>
class delegate {
public:
	using value_type = std::map<IdentifierType, CallbackType>;


	template<typename T>
	auto insert(T&& id, CallbackType callback) -> bool {
		return associations_.emplace(typename value_type::value_type{ std::forward<T>(id), callback }).second;
	}

	template<typename T>
	auto remove(T&& id) -> bool {
		return associations_.erase(std::forward<T>(id)) == 1;
	}

	auto clear() -> void {
		associations_.clear();
	}

	auto begin() -> typename value_type::const_iterator {
		return associations_.cbegin();
	}

	auto end() -> typename value_type::const_iterator {
		return associations_.cend();
	}

	template<typename T>
	auto find(T&& id, typename value_type::const_iterator& iter) -> bool {
		iter = associations_.find(std::forward<T>(id));
		if (iter != associations_.end())
			return true;

		return false;
	}

private:
	value_type associations_;
};


} // namespace okec::utils

#endif // OKEC_DELEGATE_H_