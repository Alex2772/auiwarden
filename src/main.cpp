#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AApplication.h>
#include "MainWindow.h"
#include "MyUpdater.h"

static AArc<MainWindow> gMainWindow;

AUI_ENTRY {
    // updater might relaunch the executable; handle it before taking the single instance lock
    auto updater = _new<MyUpdater>();
    updater->handleStartup(args);

    if (!AApplication::inst().requestSingleInstanceLock()) {
        // AUIwarden is already running; it has been asked to show its window.
        return 0;
    }

    auto state = _new<State>();
    state->lifetimeHold = AApplication::inst().hold();
    try {
        state->database = Database::load();
    } catch (const AException& e) {
        ALogger::warn("MainWindow") << "Can't load database: " << e;
    }

    AObject::connect(state->updateTimer->fired, state, [state = state.get()] {
        state->database.save();
    });

    AObject::connect(AApplication::inst().activated, AObject::GENERIC_OBSERVER, [=](const AActivation& activation) {
        if (!gMainWindow) {
            gMainWindow = _new<MainWindow>(state, updater);
        }
        gMainWindow->activate(activation.activationToken);
    });
    if (args.contains("--startup")) {
        if (!state->settings.showProgramWindowOnStartup) {
            return 0;
        }
    }

    gMainWindow = _new<MainWindow>(state, updater);
    gMainWindow->show();

    return 0;
};