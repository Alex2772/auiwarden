//
// Created by alex2772 on 9/2/25.
//

#include <AUI/Logging/ALogger.h>
#include "TrackerWaylandExtIdleNotifyV1.h"

using namespace std::chrono_literals;

static constexpr auto TIMEOUT = std::chrono::seconds(1min) / 2;

TrackerWaylandExtIdleNotifyV1::TrackerWaylandExtIdleNotifyV1() {
    mDisplay = wl_display_connect(nullptr);
    if (!mDisplay) {
        throw AException("Can't connect to Wayland display");
    }
    mRegistry = wl_display_get_registry(mDisplay);

    static const struct wl_registry_listener registryListener {
        .global =
            [](void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
                auto self = static_cast<TrackerWaylandExtIdleNotifyV1*>(data);
                auto interface2 = std::string_view(interface);
                ALOG_DEBUG("TrackerWaylandExtIdleNotifyV1") << interface2;
                if (interface2 == ext_idle_notifier_v1_interface.name) {
                    self->mExtIdleNotifier =
                        (ext_idle_notifier_v1*) wl_registry_bind(registry, name, &ext_idle_notifier_v1_interface, 1);
                }
                if (interface2 == wl_seat_interface.name) {
                    self->mSeat = (wl_seat*) wl_registry_bind(registry, name, &wl_seat_interface, 1);
                }
            },
        .global_remove = [](void* data, wl_registry* registry, uint32_t name) {},
    };

    wl_registry_add_listener(mRegistry, &registryListener, this);
    wl_display_roundtrip(mDisplay);

    if (mSeat == nullptr) {
        throw AException("Can't find wl_seat interface");
    }

    if (mExtIdleNotifier == nullptr) {
        throw AException("Can't find ext-idle-notify-v1 interface");
    }

    static const ext_idle_notification_v1_listener idleListener {
        .idled = [](void* data,
                    ext_idle_notification_v1* notification) {
            auto self = static_cast<TrackerWaylandExtIdleNotifyV1*>(data);
            self->mLastIdle = Idle::AWAY_FROM_KEYBOARD;
            ALOG_DEBUG("TrackerWaylandExtIdleNotifyV1") << "idled";
        },
        .resumed =
            [](void* data, ext_idle_notification_v1* notification) {
                auto self = static_cast<TrackerWaylandExtIdleNotifyV1*>(data);
                self->mLastIdle = Idle::USER_PRESENT;
              ALOG_DEBUG("TrackerWaylandExtIdleNotifyV1") << "resumed";
            },
    };
    auto notification = ext_idle_notifier_v1_get_idle_notification(mExtIdleNotifier, std::chrono::duration_cast<std::chrono::milliseconds>(TIMEOUT).count(), mSeat);
    if (notification == nullptr) {
        throw AException("Can't get idle notification");
    }
    if (ext_idle_notification_v1_add_listener(notification, &idleListener, this) != 0) {
        throw AException("Can't add idle notification listener");
    }

    mThread = _new<AThread>([this] {
        for (;;) {
            AThread::interruptionPoint();
            wl_display_dispatch(mDisplay);
        }
    });
    mThread->start();
}

TrackerWaylandExtIdleNotifyV1::~TrackerWaylandExtIdleNotifyV1() { wl_display_disconnect(mDisplay); }

void TrackerWaylandExtIdleNotifyV1::getCurrentActivity(ITracker::Activity& activity) {
    activity.idle = mLastIdle;
}
