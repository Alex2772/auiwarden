#pragma once

#include "Tracker.h"
#include "model/Idle.h"

#include <wayland-client.h>
#include <wayland-generated/protocols/ext-idle-notify-v1.xml.h>

class TrackerWaylandExtIdleNotifyV1 : public ITracker {
public:
    TrackerWaylandExtIdleNotifyV1();
    ~TrackerWaylandExtIdleNotifyV1() override;
    void getCurrentActivity(Activity& activity) override;

private:
    wl_display* mDisplay = nullptr;
    wl_registry* mRegistry = nullptr;
    ext_idle_notifier_v1* mExtIdleNotifier = nullptr;
    wl_seat* mSeat;
    _<AThread> mThread;

    Idle mLastIdle = Idle::USER_PRESENT;
};
