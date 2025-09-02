//
// Created by alex2772 on 8/31/25.
//

#include <range/v3/view/reverse.hpp>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "Tracker.h"

using namespace std::chrono;

TrackerManager::TrackerManager(_<State> state): mTrackers(getNativeTrackers()), mState(std::move(state)) {
    connect(mState->currentTime, me::update);
}

void TrackerManager::update() {
    for (const auto& tracker: mTrackers) {
        if (auto result = tracker->getCurrentActivity(); !result.empty()) {
            mState->database.handleEvent(*mState->currentTime, std::move(result));
            return;
        }
    }
}
