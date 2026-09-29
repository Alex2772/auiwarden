#pragma once

#include <AUI/Platform/AWindow.h>
#include "model/State.h"
#include "MyUpdater.h"

class SettingsWindow: public AWindow {
public:
    SettingsWindow(_<State> state, _<MyUpdater> updater, AWindow* parent);
    void onCloseButtonClicked() override;

private:
    _<State> mState;
};
