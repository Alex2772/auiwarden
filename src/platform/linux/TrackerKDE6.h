#pragma once

#include <AUI/IO/APath.h>
#include <AUI/Platform/linux/ADBus.h>
#include "Tracker.h"

class TrackerKDE6 : public ITracker {
public:
    TrackerKDE6();
    ~TrackerKDE6() override;
    void getCurrentActivity(Activity& activity) override;

private:
    AString mScriptName = "auiwarden_{}";
    APathOwner mTempScript {APath::nextRandomTemporary()};
    AString mLastData;
    ADBus::MessageFilter::Ownership mFilter{nullptr, nullptr};

    void injectScript();
};
