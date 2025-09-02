//
// Created by alex2772 on 9/2/25.
//

#include "SettingsWindow.h"
#include "Groups.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/View/ATabView.h>
#include <AUI/View/AText.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AForEachUI.h>

using namespace declarative;
using namespace ass;


SettingsWindow::SettingsWindow(_<State> state, AWindow *parent)
  : AWindow("Settings", 500_dp, 400_dp, parent, WindowStyle::MODAL), mState(std::move(state)) {
    auto tabHost = _new<ATabView>();

    tabHost->addTab(
        _new<Groups>(mState),
        "Grouping");
    tabHost->setExpanding();

    setContents(Vertical::Expanding {
      tabHost,
    });
}

void SettingsWindow::onCloseButtonClicked() {
    AWindow::onCloseButtonClicked();
    mState->database.save();
}
