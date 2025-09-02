#pragma once

#include <AUI/Common/ATimer.h>
#include "model/Database.h"
#include "model/State.h"

class ITracker {
public:
    virtual ~ITracker() = default;
    virtual AString getCurrentActivity() = 0;
};

class TrackerManager: public AObject {
public:
    TrackerManager(_<State> state);

private:
    AVector<_<ITracker>> mTrackers;
    _<State> mState;

    static AVector<_<ITracker>> getNativeTrackers();

    void update();
};
