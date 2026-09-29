#pragma once

#include <AUI/View/AViewContainer.h>
#include "MyUpdater.h"

class About : public AViewContainer {
public:
    explicit About(_<MyUpdater> updater);

private:
    _<MyUpdater> mUpdater;
};
