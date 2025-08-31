#pragma once

#include <AUI/Common/AVector.h>
#include "TimeSpan.h"

struct Database {
    struct Day {
        std::chrono::time_point<std::chrono::system_clock, std::chrono::days> timepoint;
        AVector<_<TimeSpan>> spans;
    };
    AVector<Day> days;

    static Database load();
    void save();
};
