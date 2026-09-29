#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AApplication.h>
#include "MainWindow.h"
#include "MyUpdater.h"

AUI_ENTRY {
    // updater might relaunch the executable; handle it before taking the single instance lock
    auto updater = _new<MyUpdater>();
    updater->handleStartup(args);

    if (!AApplication::inst().requestSingleInstanceLock()) {
        // AUIwarden is already running; it has been asked to show its window.
        return 0;
    }

    auto window = _new<MainWindow>(std::move(updater));

    // `--background`: start tracking without showing the window (i.e., autostart). The window is kept alive (and the
    // application is held) until the user launches AUIwarden again; after that, closing the window quits the app as
    // usual.
    struct Background {
        _<MainWindow> window;
        _<AApplication::Hold> hold = AApplication::inst().hold();
    };
    static AOptional<Background> background;
    if (args.contains("--background")) {
        background = Background { .window = window };
    } else {
        window->show();
    }

    AObject::connect(AApplication::inst().activated, window, [window = window.get()](const AActivation& activation) {
        window->activate(activation.activationToken);
        background.reset();   // the shown window holds the application from now on
    });
    return 0;
};