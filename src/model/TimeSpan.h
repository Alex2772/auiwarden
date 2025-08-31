#pragma once

#include <chrono>
#include <variant>
#include <AUI/Common/AString.h>

struct TimeSpan {
    using Timepoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>;
    Timepoint begin;
    Timepoint end;
    AString title;
};