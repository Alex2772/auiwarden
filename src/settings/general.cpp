//
// Created by alex2772 on 9/30/26.
//

#include "general.h"

#include "platform/Autostart.h"

#include "AUI/Platform/AApplication.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Util/Declarative/Containers.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/AGroupBox.h"
#include "AUI/View/ARadioButton.h"

using namespace declarative;
using namespace ass;

_<AView> settings::tab::generalView(AArc<Settings> settings) {
    platform::isAutostartEnabled.invalidate();
    return Vertical {
        GroupBox {
            CheckBox {
                .checked = AUI_REACT(*platform::isAutostartEnabled),
                .onCheckedChange = [](bool value) {
                    try {
                        platform::setAutostartEnabled(value);
                    } catch (const std::exception& e) {
                        ALogger::err("Autostart") << "unable to change autostart: " << e.what();
                    }
                },
                .content = Label { "Add program to startup" },
            },
            Vertical {
                CheckBox {
                    .checked = AUI_REACT(settings->showProgramWindowOnStartup),
                    .onCheckedChange = [settings](bool value) {
                        settings->showProgramWindowOnStartup = value;
                    },
                    .content = Label { "Show program window on startup" },
                },
            },
        },
        GroupBox {
            Label { "When I close app window:" },
            Vertical {
                RadioButton {
                    .checked = AUI_REACT(settings->allowBackgroundWork),
                    .onClick = [settings] {
                        settings->allowBackgroundWork = true;
                    },
                    .content = Label { "continue working in background" },
                },
                RadioButton {
                    .checked = AUI_REACT(!settings->allowBackgroundWork),
                    .onClick = [settings] {
                        settings->allowBackgroundWork = false;
                    },
                    .content = Label { "quit application and stop collecting data" },
                },
            },
        },
        Horizontal {
            Button {
                .content = Label { "Open app dir..." },
                .onClick = [settings] {
                    settings->save();
                    APlatform::openUrl(AApplication::inst().dataDir());
                },
            },
        },
    } AUI_OVERRIDE_STYLE { LayoutSpacing{ 4_dp }};
}
