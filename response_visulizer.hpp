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
}

#endif // OKEC_RESPONSE_VISUALIZER_H