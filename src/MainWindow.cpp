#include "MainWindow.h"
#include "view/GridView.h"
#include "view/TimeSpanView.h"
#include "model/Database.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AProgressBar.h>
#include <AUI/View/AScrollArea.h>
#include <model/TimeSpan.h>

using namespace declarative;

MainWindow::MainWindow(_<MyUpdater> updater) : AWindow("AUIwarden", 700_dp, 500_dp), mUpdater(std::move(updater)) {
    using namespace std::chrono_literals;

    try {
        mState->database = Database::load();
    } catch (const AException& e) {
        ALogger::warn("MainWindow") << "Can't load database: " << e;
    }

    auto gridView = _new<GridView>(mState) AUI_WITH_STYLE { Expanding() };
    setContents(Vertical {
      Horizontal {
        _new<ASpacerExpanding>(2),
        weekDay(std::chrono::Monday),
        weekDay(std::chrono::Tuesday),
        weekDay(std::chrono::Wednesday),
        weekDay(std::chrono::Thursday),
        weekDay(std::chrono::Friday),
        weekDay(std::chrono::Saturday),
        weekDay(std::chrono::Sunday),
      },
      AScrollArea::Builder()
          .withContents(Stacked::Expanding {
            gridView,
            weekContent(gridView, mState->database),
          } AUI_WITH_STYLE { MinSize(600_dp, 400_dp) })
          .build() AUI_WITH_STYLE { Expanding() },
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
