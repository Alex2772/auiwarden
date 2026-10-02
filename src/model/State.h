#pragma once

#include <AUI/Common/APropertyPrecomputed.h>
#include <AUI/Common/ATimer.h>
#include "Database.h"
#include "MyUpdater.h"
#include "Settings.h"
#include "AUI/Platform/AApplication.h"

struct State: public AObject {
    State() {
        setSlotsCallsOnlyOnMyThread(true);
        connect(updateTimer->fired, [this]() {
            currentTime.invalidate();
        });
        updateTimer->start();
    }

    Database database;
    Settings settings = Settings::load();
    APropertyPrecomputed<TimeSpan::Timepoint> currentTime = [] {
        return floor<std::chrono::minutes>(std::chrono::system_clock::now());
    };

    _<AApplication::Hold> lifetimeHold;

    enum class Page {
        MAIN,
        PIE,
    };
    AProperty<Page> currentPage = Page::MAIN;

    _<ATimer> updateTimer = _new<ATimer>(std::chrono::seconds(60));
};
