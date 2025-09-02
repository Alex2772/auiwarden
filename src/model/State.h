#pragma once

#include <AUI/Common/APropertyPrecomputed.h>
#include <AUI/Common/ATimer.h>
#include "Database.h"

struct State: public AObject {
    State() {
        setSlotsCallsOnlyOnMyThread(true);
        connect(updateTimer->fired, [this]() {
            currentTime.invalidate();
        });
        updateTimer->start();
    }

    Database database;
    APropertyPrecomputed<TimeSpan::Timepoint> currentTime = [] {
        return floor<std::chrono::minutes>(std::chrono::system_clock::now());
    };

    _<ATimer> updateTimer = _new<ATimer>(std::chrono::minutes(1));
};
