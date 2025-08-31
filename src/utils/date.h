#pragma once

#include <chrono>

namespace utils::date {
std::chrono::time_point<std::chrono::system_clock, std::chrono::days> getLastWeekdayUpTo(
    std::chrono::weekday weekday, std::chrono::year_month_day pivot);
}   // namespace utils::date