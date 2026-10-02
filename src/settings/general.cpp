//
// Created by alex2772 on 9/30/26.
//

#include "general.h"

#include "ai/ClaudeSkill.h"
#include "platform/Autostart.h"

#include "AUI/Platform/AApplication.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Util/Declarative/Containers.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/AGroupBox.h"
#include "AUI/View/ARadioButton.h"
#include "AUI/View/ASpacerFixed.h"
#include "AUI/View/AText.h"

using namespace declarative;
using namespace ass;

static AArc<AView> description(AString text) {
    return AText::fromString(std::move(text)) AUI_OVERRIDE_STYLE { Opacity { 0.6f } };
}

_<AView> settings::tab::generalView(AArc<Settings> settings) {
    platform::isAutostartEnabled.invalidate();
    aislop::isClaudeSkillInstalled.invalidate();
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
                    .content = Vertical {
                        Label { "continue working in background" },
                    },
                },
#if AUI_PLATFORM_WIN || AUI_PLATFORM_LINUX
                description("You can force-quit the application by pressing CTRL+Q."),
#elif AUI_PLATFORM_MACOS
                description("You can force-quit the application by pressing ⌘Q."),
#endif
                SpacerFixed { 4_dp },
                RadioButton {
                    .checked = AUI_REACT(!settings->allowBackgroundWork),
                    .onClick = [settings] {
                        settings->allowBackgroundWork = false;
                    },
                    .content = Label { "quit application and stop collecting data" },
                },
            },
        },
        GroupBox {
            Label { "AI Slop" },
            Vertical {
                CheckBox {
                    .checked = AUI_REACT(*aislop::isClaudeSkillInstalled),
                    .onCheckedChange = [](bool value) {
                        try {
                            aislop::setClaudeSkillInstalled(value);
                        } catch (const std::exception& e) {
                            ALogger::err("AI Slop") << "unable to change Claude skill: " << e.what();
                        }
                    },
                    .content = Label { "Let Claude access AUIwarden data" },
                },
                description("Installs a skill to ~/.claude/skills/auiwarden, so you can ask Claude i.e. \"summarize how I spent my week and give recommendations\". Your data is sent to Claude only when you ask."),
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
