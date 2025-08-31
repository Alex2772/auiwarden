#pragma once

#include <AUI/Common/AVector.h>
#include "TimeSpan.h"

struct Database {
    using Timepoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::days>;
    struct Day {
        Timepoint timepoint;
        AVector<_<TimeSpan>> spans;
    };
    AVector<_<Day>> days;

    static Database load();
    void save();
    void handleEvent(std::chrono::system_clock::time_point timepoint, AOptional<AString> activity);

};
