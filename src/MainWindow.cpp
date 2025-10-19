#include "MainWindow.h"
#include "view/GridView.h"
#include "model/Database.h"
#include "PageMain.h"
#include "PagePie.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AProgressBar.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <settings/SettingsWindow.h>
#include <model/TimeSpan.h>
#include <AUI/View/AHDividerView.h>

#if __has_include(<AUI/Remote/AHotCodeReload.h>)
#include <AUI/Remote/AHotCodeReload.h>
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
#if __has_include(<AUI/Remote/AHotCodeReload.h>)
    connect(AHotCodeReload::inst().patchEnd, me::inflate);
#endif
    inflate();
}

namespace {

_<AView> leftBarButton(_<AView> content, aui::invocable auto && onClick) {
    content->setExpanding();
    return Button {
        .content = content,
        .onClick = std::move(onClick)
    } AUI_WITH_STYLE {
        MinSize { 0, 0 },
        FixedSize { 32_dp, 32_dp },
    };
}

_<AView> pageButton(const _<State>& state, _<AView> content, State::Page page) {
    return leftBarButton(content, [=] {
        state->currentPage = page;
    });
}
}

void MainWindow::inflate() {
    setContents(Horizontal {
      Vertical {
        pageButton(mState, Icon { ":img/house.svg" }, State::Page::MAIN),
        pageButton(mState, Icon { ":img/pie.svg" }, State::Page::PIE),
        _new<AHDividerView>(),
        SpacerExpanding { },
        leftBarButton(Icon { ":img/settings.svg" }, [this] { _new<SettingsWindow>(mState, this)->show(); }),
      } AUI_WITH_STYLE { Padding { 4_dp }, LayoutSpacing { 4_dp } },
      CustomLayout::Expanding {} & mState->currentPage.readProjected([this](State::Page page) -> _<AView> {
          switch (page) {
              case State::Page::MAIN:
                  return views::pageMain(mState);
              case State::Page::PIE:
                  return views::pagePie(mState);
          }
      }),
    } AUI_WITH_STYLE { LayoutSpacing { 4_dp } });
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
