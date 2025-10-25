#pragma once

#include "Tracker.h"

class WindowsTracker : public ITracker {
public:
    ~WindowsTracker() override = default;
    void getCurrentActivity(Activity& activity) override;
};
