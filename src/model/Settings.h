#pragma once
#include "AUI/Common/AProperty.h"

struct Settings {
    AProperty<bool> showProgramWindowOnStartup = false;
    AProperty<bool> allowBackgroundWork = true;

    static Settings load();
    void save();
};
