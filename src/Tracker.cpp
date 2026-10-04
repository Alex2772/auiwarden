//
// Created by alex2772 on 8/31/25.
//

#include <range/v3/view/reverse.hpp>
#include <fmt/chrono.h>

#include <AUI/Common/SharedPtr.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "Tracker.h"

#include "AUI/AppInfo.h"
#include "AUI/Platform/AMessageBox.h"

using namespace std::chrono;

TrackerManager::TrackerManager(_<State> state): mTrackers(getNativeTrackers()), mState(std::move(state)) {
    connect(mState->currentTime, me::update);

    if (mTrackers.empty()) {
        AMessageBox::show(nullptr, aui::app_info::name, "Your desktop environment is not supported. Supported DEs: GNOME, KDE.");
    }
}

void TrackerManager::update() {
    ITracker::Activity activity;
    for (const auto& tracker: mTrackers) {
        tracker->getCurrentActivity(activity);
    }
    const auto currentTime = *mState->currentTime;
    ALogger::info("Database") << "timepoint={}, activity.idle={}, activity.activeWindowTitle={}"_format(currentTime, activity.idle, activity.activeWindowTitle);
    if (activity.idle == Idle::AWAY_FROM_KEYBOARD) {
        mState->database.handleAfk();
        return;
    }
    mState->database.handleEvent(currentTime, std::move(activity.activeWindowTitle));
}
