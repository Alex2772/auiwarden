//
// Created by alex2772 on 9/1/25.
//

#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Platform/linux/ADBus.h>
#include <AUI/Platform/AProcess.h>
#include <AUI/Util/kAUI.h>
#include "TrackerKDE6.h"

TrackerKDE6::TrackerKDE6(): mScriptName("auiwarden_{}"_format(AProcess::self()->getPid())) {
    mTempScript.value().parent().makeDirs();

    auto myUniqueName = ADBus::session().uniqueName();
    ALOG_DEBUG("TrackerKDE6") << "My unique name: " << myUniqueName;
    AFileOutputStream(mTempScript) << R"(
function output_debug(message) {
print("auiwarden DEBUG", message);
}

function output_error(message) {
    print("auiwarden ERROR", message);
    callDBus(")" << myUniqueName << R"(", "/", "", "error", message.toString());
}

function output_result(message) {
    if (message == null) {
        message = "null";
    }
    callDBus(")" << myUniqueName << R"(", "/", "", "result", message.toString());
}

workspace_windowList                  = () => workspace.windowList();
workspace_activeWindow                = () => workspace.activeWindow;
workspace_setActiveWindow             = (window) => { workspace.activeWindow = window; };
workspace_raiseWindow                 = (window) => { workspace.raiseWindow(window); };
workspace_currentDesktop              = () => workspace.currentDesktop.x11DesktopNumber;
workspace_setCurrentDesktop           = (id) => {
    let d = workspace.desktops.find((d) => d.x11DesktopNumber == id);
    if (d) {
        workspace.currentDesktop = d;
    } else {
        output_error(`Invalid desktop number ${id}`);
    }
};
workspace_numDesktops                 = () => workspace.desktops.length;
workspace_setNumDesktops              = (n) => { output_error("`set_num_desktops` unsupported in KDE 6"); };
window_x11DesktopIds                  = (window) => window.desktops.map((d) => d.x11DesktopNumber);
window_setX11DesktopId                = (window, id) => {
    if (id < 0) {
        window.desktops = [workspace.currentDesktop];
    } else {
        let d = workspace.desktops.find((d) => d.x11DesktopNumber == id);
        if (d) {
            window.desktops = [d];
        } else {
            output_error(`Invalid desktop number ${id}`);
        }
    }
};
window_screen                         = (window) => { output_error("`search --screen` unsupported in KDE 6"); };

function run() {
    var window_stack = [];

    output_debug("STEP getactivewindow")
    var window_stack = [workspace_activeWindow()];

    for (var i = 0; i < window_stack.length; ++i) {
        output_result(window_stack[i].caption);
    }

}

run();
    )";
    injectScript();

    mFilter = ADBus::session().addFilter([this, myUniqueName](DBusMessage* msg) {
        auto destination = dbus_message_get_destination(msg);
        if (destination == nullptr) {
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }
        if (destination != myUniqueName) {
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }
        if (dbus_message_is_method_call(msg, "", "result")) {
            DBusError error{};

            DBusMessageIter args{};
            if (!dbus_message_iter_init(msg, &args)) {
                throw AException("dbus_message_iter_init failed");
            }
            auto result = aui::dbus::iter_get<AString>(&args);
            ALOG_DEBUG("TrackerKDE6") << "Got result: " << result;
            AThread::main()->enqueue([this, result = std::move(result)]() mutable {
                mLastData = std::move(result);
            });
        }
        if (dbus_message_is_method_call(msg, "", "error")) {
            DBusError error {};

            DBusMessageIter args {};
            if (!dbus_message_iter_init(msg, &args)) {
                throw AException("dbus_message_iter_init failed");
            }
            auto result = aui::dbus::iter_get<AString>(&args);
            ALogger::err("TrackerKDE6") << "Got error: " << result;
        }

        return DBUS_HANDLER_RESULT_HANDLED;
    });
}

void TrackerKDE6::injectScript() {
    auto scriptId = *ADBus::session().callWithResult<int32_t>(
        "org.kde.KWin", // bus
        "/Scripting", // object
        "org.kde.kwin.Scripting", // interface
        "loadScript", // method
        mTempScript.value().toStdString(), mScriptName);
    if (scriptId < 0) {
        throw AException("Failed to load script");
    }
    ALogger::info("TrackerKDE6") << "Script loaded with id: " << scriptId;

    *ADBus::session().callWithResult<void>(
        "org.kde.KWin",                            // bus
        "/Scripting/Script{}"_format(scriptId),    // object
        "org.kde.kwin.Script",                     // interface
        "run");

    *ADBus::session().callWithResult<void>(
        "org.kde.KWin",                            // bus
        "/Scripting/Script{}"_format(scriptId),    // object
        "org.kde.kwin.Script",                     // interface
        "stop");

    ADBus::session().call(
        "org.kde.KWin", // bus
        "/Scripting", // object
        "org.kde.kwin.Scripting", // interface
        "unloadScript", // method
        mScriptName);
}

TrackerKDE6::~TrackerKDE6() {

}

AString TrackerKDE6::getCurrentActivity() {
    try {
        injectScript();

        return mLastData;
    } catch (const AException& e) {
        ALogger::err("TrackerKDE6") << "Can't get current activity: " << e;
        throw;
    }
}
