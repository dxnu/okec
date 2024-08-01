///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_VISUALIZER_HPP_
#define OKEC_VISUALIZER_HPP_


// #include "matplotlibcpp.h"


namespace okec
{
    // namespace plt = matplotlibcpp;

    template <typename T>
    void draw(const std::vector<T>& list, std::string_view ylabel) {
    //     plt::plot(list);
    //     // plt::xlabel("tasks");
    //     plt::ylabel(ylabel.data());
    //     plt::show();
    }

    // template <typename T, typename U>
    // void draw(const std::vector<T>& x_points, const std::vector<U>& y_points, std::string_view xlabel, std::string_view ylabel) {
    //     plt::plot(x_points, y_points);
    //     plt::xlabel(xlabel.data());
    //     plt::ylabel(ylabel.data());
    //     plt::show();
    // }
}

#endif // OKEC_VISUALIZER_HPP_