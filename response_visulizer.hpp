#ifndef OKEC_RESPONSE_VISUALIZER_H
#define OKEC_RESPONSE_VISUALIZER_H

#include "matplotlibcpp.h"

namespace okec
{
    namespace plt = matplotlibcpp;

    template <typename T>
    void draw(const std::vector<T>& list, std::string_view ylabel) {
        plt::plot(list);
        // plt::xlabel("tasks");
        plt::ylabel(ylabel.data());
        plt::show();
    }

    template <typename T, typename U>
    void draw(const std::vector<T>& x_points, const std::vector<U>& y_points, std::string_view xlabel, std::string_view ylabel) {
        plt::plot(x_points, y_points);
        plt::xlabel(xlabel.data());
        plt::ylabel(ylabel.data());
        plt::show();
    }
}

#endif // OKEC_RESPONSE_VISUALIZER_H