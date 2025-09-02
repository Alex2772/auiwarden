#pragma once

#include <AUI/Platform/AWindow.h>
#include "model/State.h"

class Groups: public AViewContainer {
public:
    Groups(_<State> state);

private:
    AProperty<_<Group>> mSelectedGroup;
};
