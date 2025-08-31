#pragma once

#include <chrono>
#include <variant>
#include <AUI/Common/AString.h>

struct TimeSpan {
    std::chrono::minutes duration;
    AOptional<AString> title;
};