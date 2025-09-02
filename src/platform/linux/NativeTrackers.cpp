#include <Tracker.h>
#include "TrackerKDE6.h"

AVector<_<ITracker>> TrackerManager::getNativeTrackers() {
    return {
        _new<TrackerKDE6>(),
    };
}
