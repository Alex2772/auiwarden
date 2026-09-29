#include <Tracker.h>
#include "TrackerKDE6.h"
#include "TrackerGnome.h"
#include "TrackerWaylandExtIdleNotifyV1.h"

template<aui::derived_from<ITracker> T>
static void tryInitialize(AVector<_<ITracker>>& out) {
    try {
        out << _new<T>();
        ALogger::info("TrackerManager") << "Installed: " << AClass<T>::name();
    } catch (const AException& e) {
        ALogger::warn("TrackerManager") << AClass<T>::name() << " init: " << e;
    }
}

AVector<_<ITracker>> TrackerManager::getNativeTrackers() {
    AVector<_<ITracker>> out;
    tryInitialize<TrackerKDE6>(out);
    tryInitialize<TrackerGnome>(out);
    tryInitialize<TrackerWaylandExtIdleNotifyV1>(out);
    return out;
}
