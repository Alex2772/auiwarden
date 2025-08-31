#include <AUI/Platform/Entry.h>
#include "MainWindow.h"
#include "MyUpdater.h"

AUI_ENTRY {
    auto updater = _new<MyUpdater>();
    updater->handleStartup(args);

    _new<MainWindow>(std::move(updater))->show();
    return 0;
};