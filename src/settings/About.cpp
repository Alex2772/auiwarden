#include "About.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Util/kAUI.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AText.h>

#include "AUI/View/ADrawableView.h"
#include "AUI/View/ASpacerFixed.h"

using namespace declarative;
using namespace ass;

static AString stateToString(const std::any& status) {
    if (auto* value = std::any_cast<AUpdater::StatusCheckingForUpdates>(&status)) {
        return "Checking for updates...";
    }
    if (auto* value = std::any_cast<AUpdater::StatusDownloading>(&status)) {
        return "Downloading...";
    }
    if (auto* value = std::any_cast<AUpdater::StatusNotAvailable>(&status)) {
        return "Not available";
    }
    return " ";
}

About::About(_<MyUpdater> updater) : mUpdater(std::move(updater)) {
    setContents(Centered::Expanding{
        Vertical{
            Centered{
                Icon{":img/icon.svg"} AUI_OVERRIDE_STYLE{FixedSize(128_dp)},
            },
            SpacerFixed{4_dp},
            Label{"AUIwarden"} AUI_OVERRIDE_STYLE{
                FontSize{19_pt},
                //Margin { 0, 0, 4_dp },
                ATextAlign::CENTER,
                // TextColor{0x444444_rgb},
            },
            Label{"Built " __DATE__ " " __TIME__} AUI_OVERRIDE_STYLE{
                FontSize{8_pt},
                //Margin { 0, 0, 4_dp },
                ATextAlign::CENTER,
                TextColor{0x444444_rgb},
            },
            Label{"Version: {}"_format(AUI_PP_STRINGIZE(AUI_CMAKE_PROJECT_VERSION))} AUI_OVERRIDE_STYLE{
                FontSize{8_pt},
                //Margin { 0, 0, 4_dp },
                ATextAlign::CENTER,
                TextColor{0x444444_rgb},
            },
            SpacerFixed{8_dp},
            Centered{
                // SpacerExpanding(),
                Button{
                    Label{ "Check for updates..." },
                    [this] { mUpdater->checkForUpdates(); },
                },
            },
            Label { AUI_REACT(stateToString(mUpdater->status)) },
            SpacerFixed{16_dp},
        }
    } AUI_OVERRIDE_STYLE{LayoutSpacing{4_dp}});
}
