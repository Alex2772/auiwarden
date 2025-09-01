#pragma once

#include <chrono>
#include <AUI/View/AView.h>
#include "model/TimeSpan.h"
#include "model/Database.h"
#include "GridView.h"

namespace declarative {
_<AView> weekDay(std::chrono::weekday day);
_<AView> activityRectangle(std::chrono::minutes duration, _<AView> view);
_<AView> weekContent(const _<GridView>& gridView, const Database& db);

}