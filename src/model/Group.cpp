//
// Created by alex2772 on 9/2/25.
//
#include "Group.h"
#include <random>

AColor Group::nextRandomColor() {
    static std::default_random_engine re;
    re.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    return operator""_rgb(std::uniform_int_distribution<uint32_t>(0, 0xFFFFFF)(re));
}
