#pragma once

#include <AUI/Platform/linux/ADBus.h>
#include "Tracker.h"

/**
 * @brief Active window tracker for GNOME (Mutter/Wayland) without any extensions.
 *
 * GNOME Shell exposes the active window via org.gnome.Shell.Introspect.GetWindows, but allows only the owners of
 * org.freedesktop.impl.portal.desktop.gtk/gnome bus names to call it. We take over such a name (REPLACE_EXISTING) for
 * the duration of a single request, then release it, so the original portal backend can be activated again by the bus.
 */
class TrackerGnome : public ITracker {
public:
    TrackerGnome();
    ~TrackerGnome() override;
    void getCurrentActivity(Activity& activity) override;

private:
    AString mLastTitle;

    /**
     * @return title of the focused window (may be empty).
     */
    AString queryFocusedWindowTitle();
};
