#pragma once

#include <AUI/Platform/AWindow.h>
#include "MyUpdater.h"
#include "model/Database.h"
#include "model/State.h"
#include "Tracker.h"

class MainWindow: public AWindow {
public:
    MainWindow(_<MyUpdater> updater);
    void onCloseButtonClicked() override;

private:
    _<MyUpdater> mUpdater;
    _<State> mState = _new<State>();
    TrackerManager mTrackerManager = mState;

    void inflate();
    void save();
};
