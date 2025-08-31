#include "utils/date.h"
#include <gtest/gtest.h>
#include <AUI/Traits/strings.h>
#include <fmt/format.h>

using namespace std::chrono;
using namespace std::chrono_literals;


TEST(DateUtils, Case1) {
    //
    // This is want i want to accomplish
    //
    //                    imaginaryToday
    //                      VVVVVVVVVV
    // | 18 Aug | 19 Aug  | 20 Aug    | 14 Aug   | 15 Aug | 16 Aug   | 17 Aug |
    // | Monday | Tuesday | Wednesday | Thursday | Friday | Saturday | Sunday |
    // |--------|---------|-----------|----------|--------|----------|--------|


    const auto imaginaryToday = 2025y/August/20;
    EXPECT_EQ(imaginaryToday, 2025y/August/20);

    EXPECT_EQ(utils::date::getLastWeekdayUpTo(Monday, imaginaryToday), 2025y/August/18);
    EXPECT_EQ(utils::date::getLastWeekdayUpTo(Tuesday, imaginaryToday), 2025y/August/19);
    EXPECT_EQ(utils::date::getLastWeekdayUpTo(Wednesday, imaginaryToday), 2025y/August/20);
    EXPECT_EQ(utils::date::getLastWeekdayUpTo(Thursday, imaginaryToday), 2025y/August/14);
    EXPECT_EQ(utils::date::getLastWeekdayUpTo(Friday, imaginaryToday), 2025y/August/15);
    EXPECT_EQ(utils::date::getLastWeekdayUpTo(Saturday, imaginaryToday), 2025y/August/16);
    EXPECT_EQ(utils::date::getLastWeekdayUpTo(Sunday, imaginaryToday), 2025y/August/17);
}
