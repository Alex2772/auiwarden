//
// Created by alex2772 on 9/30/26.
//

#include "Settings.h"

#include <AUI/IO/APath.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Json/Conversion.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AApplication.h>

#include "platform/Autostart.h"

AJSON_FIELDS(Settings,
             AJSON_FIELDS_ENTRY(showProgramWindowOnStartup)
             AJSON_FIELDS_ENTRY(allowBackgroundWork)
)

static constexpr auto LOG_TAG = "Settings";

static APath settingsJson() { return AApplication::inst().dataDir() / "settings.json"; }

static void onFirstLaunch() {
}

Settings Settings::load() {
    auto path = settingsJson();
    if (!path.isRegularFileExists()) {
        onFirstLaunch();
        Settings s;
        s.save();
        return s;
    }
    try {
        return aui::from_json<Settings>(AJson::fromStream(AFileInputStream(path)));
    } catch (const AException& e) {
        ALogger::warn(LOG_TAG) << "Could not load settings, using defaults: " << e;
        return {};
    }
}

void Settings::save() {
    auto path = settingsJson();
    // write to a temporary file first, then atomically replace to avoid a corrupted json on crash.
    auto tmp = path.parent() / (path.filename() + ".tmp");
    {
        AFileOutputStream(tmp) << aui::to_json(*this);
    }
    APath::move(tmp, path);
}
