///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_COLOR_H_
#define OKEC_COLOR_H_

#include <fmt/color.h>

namespace okec {

    enum class color : uint32_t {
        debug = 0xCC8BF5,            // rgb(204,139,245)
        info = 0x77F9F6,             // rgb(119,249,246)
        warning = 0xFCF669,          // rgb(252,246,105)
        success = 0x84FD61,          // rgb(132,253,97)
        error = 0xE96A63,            // rgb(233,106,99)
    };

    // inline constexpr auto info    = fmt::rgb(119,249,246);
    // inline constexpr auto warning = fmt::rgb(251,248,103);
    // inline constexpr auto debug   = fmt::rgb(204,139,245);
    // inline constexpr auto success = fmt::rgb(132,253,97);
    // inline constexpr auto error   = fmt::rgb(233,105,104);

} // namespace okec

#endif // OKEC_COLOR_H_