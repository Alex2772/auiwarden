#include "MainWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AButton.h>
#include <AUI/Platform/APlatform.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/View/AProgressBar.h>

using namespace declarative;

MainWindow::MainWindow(_<MyUpdater> updater)
  : AWindow("Project template app", 300_dp, 200_dp), mUpdater(std::move(updater)) {
    setContents(Centered { Vertical {
      Centered { Icon { ":img/icon.svg" } AUI_WITH_STYLE { FixedSize(64_dp) } },
      Centered { Label { "Hello world from AUI!" } },
      _new<AButton>("Visit GitHub repo")
          .connect(&AView::clicked, this, [] { APlatform::openUrl("https://github.com/aui-framework/aui"); }),
      _new<AButton>("Visit docs")
          .connect(&AView::clicked, this, [] { APlatform::openUrl("https://aui-framework.github.io/"); }),
      _new<AButton>("Submit an issue")
          .connect(
              &AView::clicked, this, [] { APlatform::openUrl("https://github.com/aui-framework/aui/issues/new"); }),
      CustomLayout {} & mUpdater->status.readProjected([&updater = mUpdater](const std::any& status) -> _<AView> {
          if (std::any_cast<AUpdater::StatusIdle>(&status)) {
              return _new<AButton>("Check for updates").connect(&AView::clicked, AUI_SLOT(updater)::checkForUpdates);
          }
          if (std::any_cast<AUpdater::StatusCheckingForUpdates>(&status)) {
              return Label { "Checking for updates..." };
          }
          if (auto downloading = std::any_cast<AUpdater::StatusDownloading>(&status)) {
              return Vertical {
                  Label { "Downloading..." },
                  _new<AProgressBar>() & downloading->progress,
              };
          }
          if (std::any_cast<AUpdater::StatusWaitingForApplyAndRestart>(&status)) {
              return _new<AButton>("Apply update and restart")
                  .connect(&AView::clicked, AUI_SLOT(updater)::applyUpdateAndRestart);
          }
          return nullptr;
      }),
      Label { "Btw, 2 + 2 = {}"_format(sum(2, 2)) },
      Label { "Version: " AUI_PP_STRINGIZE(AUI_CMAKE_PROJECT_VERSION) },
    } });
}

int MainWindow::sum(int a, int b) { return a + b; }
