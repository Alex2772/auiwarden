#pragma once

#include <AUI/Common/AVector.h>
#include <AUI/Common/AProperty.h>
#include "TimeSpan.h"
#include "Group.h"

struct Database {
    /*
    using Timepoint = std::chrono::time_point<std::chrono::system_clock, std::chrono::days>;
    struct Day {
        Timepoint timepoint;
    };
    AVector<_<Day>> days;
    */
    AProperty<AVector<_<TimeSpan>>> spans;
    AProperty<AVector<_<Group>>> groups = AVector<_<Group>>{
        aui::ptr::manage_shared(new Group{
            .name = "Web",
            .windowTitleContains = "Chrome\nFirefox\nOpera\nVivaldi\nBrave\nYandex\nSafari\nEdge\nTor Browser\nBrave Browser\nChromium\nUranium",
        }),
        aui::ptr::manage_shared(new Group{
            .name = "Gaming",
            .windowTitleContains = "Dota\nOverwatch",
        }),
        aui::ptr::manage_shared(new Group{
          .name = "Work",
          .windowTitleContains = "CLion\nVisual Studio",
        }),
    };

    static Database load();
    void save();
    void handleEvent(TimeSpan::Timepoint timepoint, AString activity);

};
