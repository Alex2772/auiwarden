#pragma once

#include <AUI/Common/AProperty.h>
#include <AUI/Common/AString.h>

struct Group {
    AProperty<AString> name;
    AProperty<AColor> color = nextRandomColor();
    AProperty<AString> windowTitleContains;

    static AColor nextRandomColor();
};
