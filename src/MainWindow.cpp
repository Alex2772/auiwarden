#include "MainWindow.h"
#include "view/GridView.h"
#include "view/TimeSpanView.h"
#include "model/Database.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AProgressBar.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/View/AForEachUI.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <settings/SettingsWindow.h>
#include <model/TimeSpan.h>

#if AUI_DEBUG
#include <AUI/Remote/AHotCodeReload.h>
#include <range/v3/view/iota.hpp>
#include <AUI/View/ASpacerFixed.h>
#endif

using namespace declarative;

MainWindow::MainWindow(_<MyUpdater> updater) : AWindow("AUIwarden", 700_dp, 500_dp), mUpdater(std::move(updater)) {
    using namespace std::chrono_literals;

    try {
        mState->database = Database::load();
    } catch (const AException& e) {
        ALogger::warn("MainWindow") << "Can't load database: " << e;
    }

    connect(mState->updateTimer->fired, me::save);
    connect(AHotCodeReload::inst().patchEnd, me::inflate);
    inflate();
}

void MainWindow::inflate() {
    auto gridView = _new<GridView>(mState) AUI_WITH_STYLE { Expanding() };
    setContents(Horizontal {
      Vertical {
        Button {
          .content = Icon { ":img/settings.svg" },
          .onClick = [&] { _new<SettingsWindow>(mState, this)->show(); },
        },
      },
      Vertical::Expanding {
        Horizontal {
          SpacerFixed { 40_dp },
          weekDay(std::chrono::Monday, mState),
          weekDay(std::chrono::Tuesday, mState),
          weekDay(std::chrono::Wednesday, mState),
          weekDay(std::chrono::Thursday, mState),
          weekDay(std::chrono::Friday, mState),
          weekDay(std::chrono::Saturday, mState),
          weekDay(std::chrono::Sunday, mState),
        },
        AScrollArea::Builder()
            .withContents(Horizontal {
              AUI_DECLARATIVE_FOR(i, ranges::view::iota(0, 24), AVerticalLayout) {
                  return Vertical::Expanding {
                      Label { "{}:00"_format(i) } AUI_WITH_STYLE {
                        TransformOffset { 0, 11_pt / -2.f },
                        ATextAlign::RIGHT,
                        Padding { 0, 4_dp },
                      },
                      SpacerExpanding {},
                  };
              } AUI_WITH_STYLE { FixedSize{ 40_dp, {} } },
              Stacked::Expanding {
                gridView,
                weekContent(gridView, mState),
              } AUI_WITH_STYLE { MinSize(600_dp, 400_dp) },
            })
            .build() AUI_WITH_STYLE { Expanding() },
      },
    });
}

void MainWindow::save() {
    try {
        mState->database.save();
    } catch (const AException& e) {
        ALogger::warn("MainWindow") << "Can't save database: " << e;
    }
}

void MainWindow::onCloseButtonClicked() {
    AWindow::onCloseButtonClicked();
    save();
}
