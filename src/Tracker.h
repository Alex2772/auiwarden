#pragma once

#include <AUI/Common/ATimer.h>
#include "model/Database.h"
#include "model/State.h"
#include "model/Idle.h"

class ITracker {
public:
    struct Activity {
        AOptional<Idle> idle;
        AString activeWindowTitle;
    };

    virtual ~ITracker() = default;
    virtual void getCurrentActivity(Activity& activity) = 0;
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
