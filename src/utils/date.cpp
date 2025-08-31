#include "date.h"

using namespace std::chrono;
using namespace std::chrono_literals;

std::chrono::time_point<std::chrono::system_clock, std::chrono::days>
utils::date::getLastWeekdayUpTo(std::chrono::weekday weekday, std::chrono::year_month_day pivot) {
    auto day = pivot;
    while (std::chrono::weekday(day) != weekday) {
        day = sys_days(day) - days(1);
    }
    return day;
}
