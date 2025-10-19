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
#if !AUI_DEBUG
        updateTimer->start();
#endif
    }

    Database database;
    APropertyPrecomputed<TimeSpan::Timepoint> currentTime = [] {
        return floor<std::chrono::minutes>(std::chrono::system_clock::now());
    };

    enum class Page {
        MAIN,
        PIE,
    };
    AProperty<Page> currentPage = Page::MAIN;

    _<ATimer> updateTimer = _new<ATimer>(std::chrono::seconds(60));
};
