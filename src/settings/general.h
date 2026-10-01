#pragma once
#include "AUI/View/AView.h"
#include "model/Settings.h"

namespace settings::tab {
    _<AView> generalView(AArc<Settings> settings);
}
