#pragma once

#include <AUI/Platform/AWindow.h>
#include "MyUpdater.h"
#include "model/Database.h"
#include "model/State.h"
#include "Tracker.h"

class MainWindow: public AWindow {
public:
    MainWindow(AArc<State> state, AArc<MyUpdater> updater);
    void onCloseButtonClicked() override;
    void onKeyDown(AInput::Key key) override;

private:
    AArc<State> mState;
    AArc<MyUpdater> mUpdater;
    TrackerManager mTrackerManager = mState;

    void inflate();
};
