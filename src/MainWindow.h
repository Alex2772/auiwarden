#pragma once

#include <AUI/Platform/AWindow.h>
#include "MyUpdater.h"

class MainWindow: public AWindow {
public:
    MainWindow(_<MyUpdater> updater);
    static int sum(int a, int b);

private:
    _<MyUpdater> mUpdater;
};
