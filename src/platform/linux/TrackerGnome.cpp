#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include "TrackerGnome.h"

using namespace std::chrono_literals;

namespace {
constexpr auto LOG_TAG = "TrackerGnome";

// the well-known name, which is allowed to call org.gnome.Shell.Introspect
constexpr auto ALLOWED_BUS_NAME = "org.freedesktop.impl.portal.desktop.gtk";

using WindowProperties = AMap<AString, aui::dbus::Variant>;

// a{ta{sv}}: window id -> properties
using Windows = AMap<uint64_t, WindowProperties>;

template <typename T>
const T* get(const WindowProperties& properties, const AString& key) {
    auto it = properties.find(key);
    if (it == properties.end()) {
        return nullptr;
    }
    return std::get_if<T>(&static_cast<const aui::dbus::VariantImpl&>(it->second));
}

bool isGnomeSession() {
    const char* desktop = std::getenv("XDG_CURRENT_DESKTOP");
    if (desktop == nullptr) {
        return false;
    }
    return AString(desktop).contains("GNOME");
}

struct DesktopEntry {
    std::string fileId;   // file name, e.g. "jetbrains-clion-xxx.desktop"
    std::string wmClass;  // StartupWMClass
    std::string name;     // Name
};

const std::vector<DesktopEntry>& desktopEntries() {
    static const auto entries = [] {
        namespace fs = std::filesystem;
        std::vector<fs::path> dirs;
        if (const char* home = std::getenv("XDG_DATA_HOME"); home && *home) {
            dirs.push_back(fs::path(home) / "applications");
        } else if (const char* h = std::getenv("HOME")) {
            dirs.push_back(fs::path(h) / ".local/share/applications");
        }
        std::string dataDirs = "/usr/local/share:/usr/share";
        if (const char* d = std::getenv("XDG_DATA_DIRS"); d && *d) {
            dataDirs = d;
        }
        std::istringstream ss(dataDirs);
        for (std::string dir; std::getline(ss, dir, ':');) {
            if (!dir.empty()) {
                dirs.push_back(fs::path(dir) / "applications");
            }
        }
        dirs.emplace_back("/var/lib/flatpak/exports/share/applications");
        if (const char* h = std::getenv("HOME")) {
            dirs.push_back(fs::path(h) / ".local/share/flatpak/exports/share/applications");
        }

        std::vector<DesktopEntry> result;
        for (const auto& dir : dirs) {
            std::error_code ec;
            for (fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec)) {
                if (it->path().extension() != ".desktop") {
                    continue;
                }
                std::ifstream in(it->path());
                DesktopEntry entry{.fileId = it->path().filename().string()};
                bool inSection = false;
                for (std::string line; std::getline(in, line);) {
                    if (!line.empty() && line[0] == '[') {
                        inSection = line == "[Desktop Entry]";
                        continue;
                    }
                    if (!inSection) {
                        continue;
                    }
                    if (line.starts_with("Name=") && entry.name.empty()) {
                        entry.name = line.substr(5);
                    } else if (line.starts_with("StartupWMClass=")) {
                        entry.wmClass = line.substr(15);
                    }
                }
                if (!entry.name.empty()) {
                    result.push_back(std::move(entry));
                }
            }
        }
        return result;
    }();
    return entries;
}

std::string toLower(std::string s) {
    for (auto& c : s) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return s;
}

// Resolves human-readable application name (Name= from the .desktop file). Empty if not found.
std::string resolveAppName(const std::string& appId, const std::string& wmClass) {
    const auto& entries = desktopEntries();
    if (!appId.empty()) {
        auto id = toLower(appId);
        if (!id.ends_with(".desktop")) {
            id += ".desktop";
        }
        for (const auto& e : entries) {
            if (toLower(e.fileId) == id) {
                return e.name;
            }
        }
    }
    if (!wmClass.empty()) {
        auto cls = toLower(wmClass);
        for (const auto& e : entries) {
            if (!e.wmClass.empty() && toLower(e.wmClass) == cls) {
                return e.name;
            }
        }
        for (const auto& e : entries) {
            if (toLower(e.fileId) == cls + ".desktop") {
                return e.name;
            }
        }
    }
    return {};
}

// same threshold as in TrackerWaylandExtIdleNotifyV1 and WindowsTracker
constexpr auto IDLE_TIMEOUT = 2min;

std::chrono::milliseconds queryIdleTime() {
    auto ms = *ADBus::session().callWithResult<uint64_t>(
        "org.gnome.Mutter.IdleMonitor",               // bus
        "/org/gnome/Mutter/IdleMonitor/Core",         // object
        "org.gnome.Mutter.IdleMonitor",               // interface
        "GetIdletime");                               // method
    return std::chrono::milliseconds(ms);
}

Windows getWindows() {
    return *ADBus::session().callWithResult<Windows>(
        "org.gnome.Shell",               // bus
        "/org/gnome/Shell/Introspect",   // object
        "org.gnome.Shell.Introspect",    // interface
        "GetWindows");                   // method
}
}   // namespace

TrackerGnome::TrackerGnome() {
    if (!isGnomeSession()) {
        throw AException("not a GNOME session");
    }
    // probe: fail early (and let TrackerManager skip us) if the trick doesn't work.
    mLastTitle = queryFocusedWindowTitle();
    ALogger::info(LOG_TAG) << "Initialized; focused window: " << mLastTitle;
}

TrackerGnome::~TrackerGnome() = default;

AString TrackerGnome::queryFocusedWindowTitle() {
    auto& bus = ADBus::session();

    auto reply = *bus.callWithResult<uint32_t>(
        "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "RequestName",
        AString(ALLOWED_BUS_NAME), static_cast<uint32_t>(DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE));
    if (reply != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER && reply != DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER) {
        throw AException("unable to acquire {} (reply {})"_format(ALLOWED_BUS_NAME, reply));
    }

    // Release the name no matter what, so the original portal backend can be re-activated.
    AUI_DEFER {
        try {
            ADBus::session().call(
                "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ReleaseName",
                AString(ALLOWED_BUS_NAME));
        } catch (const AException& e) {
            ALogger::err(LOG_TAG) << "Unable to release bus name: " << e;
        }
    };

    // GNOME Shell learns about the new name owner asynchronously (Gio.DBus.watch_name), so the first few attempts
    // might be denied.
    Windows windows;
    for (int attempt = 0;; ++attempt) {
        try {
            windows = getWindows();
            break;
        } catch (const AException& e) {
            if (attempt >= 10) {
                throw;
            }
            std::this_thread::sleep_for(25ms);
        }
    }

    for (const auto& [id, properties] : windows) {
        const bool* hasFocus = get<bool>(properties, "has-focus");
        if (hasFocus == nullptr || !*hasFocus) {
            continue;
        }
        std::string wmClassStr, appIdStr;
        if (const auto* wmClass = get<std::string>(properties, "wm-class")) {
            wmClassStr = *wmClass;
        }
        if (const auto* appId = get<std::string>(properties, "app-id")) {
            appIdStr = *appId;
        }
        auto resolved = resolveAppName(appIdStr, wmClassStr);
        if (resolved.empty()) {
            resolved = wmClassStr.empty() ? appIdStr : wmClassStr;
        }
        AString appName = AString::fromUtf8(resolved);
        AString title;
        if (const auto* t = get<std::string>(properties, "title")) {
            title = AString::fromUtf8(*t);
        }
        if (appName.empty()) {
            return title;
        }
        if (title.empty() || title.lowercase().contains(appName.lowercase())) {
            return title.empty() ? appName : title;
        }
        return appName + " — " + title;
    }
    return {};
}

void TrackerGnome::getCurrentActivity(ITracker::Activity& activity) {
    try {
        mLastTitle = queryFocusedWindowTitle();
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Can't get current activity: " << e;
    }
    if (!mLastTitle.empty()) {
        activity.activeWindowTitle = mLastTitle;
    }
    try {
        activity.idle = queryIdleTime() >= IDLE_TIMEOUT ? Idle::AWAY_FROM_KEYBOARD : Idle::USER_PRESENT;
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Can't get idle time: " << e;
    }
}
