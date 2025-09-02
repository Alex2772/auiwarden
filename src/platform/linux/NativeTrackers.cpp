#include <Tracker.h>
#include "TrackerKDE6.h"
#include "TrackerWaylandExtIdleNotifyV1.h"

AVector<_<ITracker>> TrackerManager::getNativeTrackers() {
    return {
        _new<TrackerKDE6>(),
        _new<TrackerWaylandExtIdleNotifyV1>(),
    };
}
