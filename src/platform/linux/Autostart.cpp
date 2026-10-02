#include "platform/Autostart.h"

#include <cstdlib>
#include <filesystem>

#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/AException.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/IO/AFileOutputStream.h"
#include "AUI/IO/APath.h"
#include "AUI/Util/kAUI.h"

namespace {

APath entryPath() {
    APath configDir;
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME"); xdg && *xdg) {
        configDir = APath(xdg);
    } else {
        configDir = APath::getDefaultPath(APath::HOME) / ".config";
    }
    return configDir / "autostart" / "ru.alex2772.auiwarden.desktop";
}

APath currentExe() {
    // AppImage: /proc/self/exe points to a temporary mount.
    if (const char* appImage = std::getenv("APPIMAGE"); appImage && *appImage) {
        return APath(appImage);
    }
    std::error_code ec;
    auto p = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (ec) {
        return {};
    }
    return APath(p.string());
}

// Desktop Entry spec: quote the argument, escape \ " ` $ inside quotes.
AString quoteExec(const APath& path) {
    AString out = "\"";
    for (char c : static_cast<const std::string&>(path)) {
        if (c == '\\') {
            out += "\\\\"; // desktop entry requires double escaping of backslash
            continue;
        }
        if (c == '"' || c == '`' || c == '$') {
            out += '\\';
        }
        out += c;
    }
    out += "\"";
    return out;
}

AString execLine(const APath& exe) { return "Exec=" + quoteExec(exe) + " --startup"; }

}   // namespace

APropertyPrecomputed<bool> platform::isAutostartEnabled = [] {
    auto path = entryPath();
    auto exe = currentExe();
    if (exe.empty() || !path.isRegularFileExists()) {
        return false;
    }
    auto content = AString::fromUtf8(AByteBuffer::fromStream(AFileInputStream(path)));
    const auto expected = execLine(exe);
    for (const auto& line : content.split('\n')) {
        if (line.startsWith("Exec=")) {
            return line == expected;
        }
    }
    return false;
};

void platform::setAutostartEnabled(bool enabled) {
    AUI_DEFER { isAutostartEnabled.invalidate(); };
    auto path = entryPath();
    if (!enabled) {
        if (path.exists()) {
            path.removeFile();
        }
        return;
    }
    auto exe = currentExe();
    if (exe.empty()) {
        throw AException("unable to determine current executable path");
    }
    path.parent().makeDirs();
    const auto content = "[Desktop Entry]\n"
                         "Type=Application\n"
                         "Name=AUIwarden\n" +
                         execLine(exe) +
                         "\n"
                         "Terminal=false\n"
                         "X-GNOME-Autostart-enabled=true\n";
    AFileOutputStream(path).write(content.data(), content.size());
}
