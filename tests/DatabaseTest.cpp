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

    const auto baseTime = floor<minutes>(sys_days(2025y/August/20) + 12h);
    database.handleEvent(baseTime, "Dota 2");
    EXPECT_EQ(database.spans[0]->title, "Dota 2");
    EXPECT_EQ(database.spans[0]->begin, sys_days(2025y/August/20) + 12h);
    EXPECT_EQ(database.spans[0]->end, sys_days(2025y/August/20) + 12h);

    database.handleEvent(baseTime + 1min, "Dota 2");
    EXPECT_EQ(database.spans[0]->end, sys_days(2025y/August/20) + 12h + 1min);

    database.handleEvent(baseTime + 6min, "Dota 2");
    EXPECT_EQ(database.spans[0]->end, sys_days(2025y/August/20) + 12h + 1min);
    EXPECT_EQ(database.spans[1]->begin, sys_days(2025y/August/20) + 12h + 6min);
    EXPECT_EQ(database.spans[1]->end, sys_days(2025y/August/20) + 12h + 6min);
}
