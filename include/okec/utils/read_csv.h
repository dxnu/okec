///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_READ_CSV_H_
#define OKEC_READ_CSV_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>


namespace okec
{

using dataset_sequence_type = std::vector<std::vector<std::string>>;

auto read_csv(std::string_view file, std::string_view type = "", std::string_view delimiter = ",")
    -> std::optional<dataset_sequence_type>;

} // namespace okec


#endif // OKEC_READ_CSV_H_