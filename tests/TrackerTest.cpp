#include "utils/date.h"
#include "model/Database.h"
#include "Tracker.h"
#include <gtest/gtest.h>
#include <AUI/Traits/strings.h>
#include <fmt/format.h>

using namespace std::chrono;
using namespace std::chrono_literals;


TEST(TrackerTest, Case1) {
    Tracker t(nullptr);
}
