#pragma once

#include <AUI/Common/AVector.h>
#include <AUI/Common/AProperty.h>
#include "TimeSpan.h"

struct Database {
    /*
    using Timepoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::days>;
    struct Day {
        Timepoint timepoint;
    };
    AVector<_<Day>> days;
    */
    AProperty<AVector<_<TimeSpan>>> spans;

    static Database load();
    void save();
    void handleEvent(TimeSpan::Timepoint timepoint, AString activity);

};
