#include "Tracker.h"
#include "WindowsTracker.h"

AVector<_<ITracker>> TrackerManager::getNativeTrackers() {
    return {
        _new<WindowsTracker>()
    };
}
