#pragma once

#include <AUI/Platform/AWindow.h>
#include "model/State.h"

class SettingsWindow: public AWindow {
public:
    SettingsWindow(_<State> state, AWindow* parent);
    void onCloseButtonClicked() override;

private:
    _<State> mState;
};
