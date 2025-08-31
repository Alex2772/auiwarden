#include "utils/date.h"
#include "model/Database.h"
#include "Tracker.h"
#include <gtest/gtest.h>
#include <AUI/Traits/strings.h>
#include <fmt/format.h>

using namespace std::chrono;
using namespace std::chrono_literals;


TEST(Database, Case1) {
    Database database;

    const auto baseTime = sys_days(2025y/August/20) + 12h;
    database.handleEvent(baseTime, "Dota 2");
    auto& day = database.days[0];
    EXPECT_EQ(day->timepoint, floor<days>(baseTime));
    EXPECT_EQ(day->spans[0]->title, std::nullopt);
    EXPECT_EQ(day->spans[0]->duration, 12h);
    EXPECT_EQ(day->spans[1]->title, "Dota 2");
    EXPECT_EQ(day->spans[1]->duration, 1min);
}
