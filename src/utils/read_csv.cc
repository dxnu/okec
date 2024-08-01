///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/utils/read_csv.h>
#include <fstream>
#include <ranges>


auto okec::read_csv(std::string_view file, std::string_view type, std::string_view delimiter)
    -> std::optional<dataset_sequence_type>
{
    std::ifstream data_file(file.data());
    if (!data_file.is_open()) {
        return {};
    }

    std::string line;
    std::getline(data_file, line); // skip the title
    dataset_sequence_type result;
    while (std::getline(data_file, line)) {
        auto tokens = line
                    | std::views::split(delimiter)
                    | std::views::transform([](auto&& token) {
                        return std::string_view(&*token.begin(), std::ranges::distance(token));
                    });
        
        auto it = std::ranges::begin(tokens);
        std::ranges::advance(it, 2);
        if (type.empty() || *it == type) {
			// save all records or filtered records.
			result.push_back(dataset_sequence_type::value_type(tokens.begin(), tokens.end()));
		}
    }

    return result;
}