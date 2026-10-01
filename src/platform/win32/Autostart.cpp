#include "platform/Autostart.h"

#include <windows.h>

#include <stdexcept>
#include <string>

#include "AUI/Util/kAUI.h"

#ifdef _MSC_VER
#pragma comment(lib, "advapi32.lib")
#endif

namespace {

constexpr auto RUN_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr auto VALUE_NAME = L"AUIwarden";

std::wstring currentExe() {
    std::wstring buf(MAX_PATH, L'\0');
    for (;;) {
        DWORD n = GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
        if (n == 0) {
            return {};
        }
        if (n < buf.size()) {
            buf.resize(n);
            return buf;
        }
        buf.resize(buf.size() * 2);
    }
}

std::wstring expectedCommand() {
    auto exe = currentExe();
    if (exe.empty()) {
        return {};
    }
    return L"\"" + exe + L"\" --startup";
}

}   // namespace

APropertyPrecomputed<bool> platform::isAutostartEnabled = [] {
    auto expected = expectedCommand();
    if (expected.empty()) {
        return false;
    }
    DWORD size = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, RUN_KEY, VALUE_NAME, RRF_RT_REG_SZ, nullptr, nullptr, &size) != ERROR_SUCCESS ||
        size == 0) {
        return false;
    }
    std::wstring value(size / sizeof(wchar_t), L'\0');
    if (RegGetValueW(HKEY_CURRENT_USER, RUN_KEY, VALUE_NAME, RRF_RT_REG_SZ, nullptr, value.data(), &size) !=
        ERROR_SUCCESS) {
        return false;
    }
    value.resize(wcsnlen(value.c_str(), value.size()));
    // case-insensitive comparison
    return CompareStringOrdinal(value.c_str(), static_cast<int>(value.size()), expected.c_str(),
                                static_cast<int>(expected.size()), TRUE) == CSTR_EQUAL;
};

void platform::setAutostartEnabled(bool enabled) {
    AUI_DEFER { isAutostartEnabled.invalidate(); };
    if (!enabled) {
        auto r = RegDeleteKeyValueW(HKEY_CURRENT_USER, RUN_KEY, VALUE_NAME);
        if (r != ERROR_SUCCESS && r != ERROR_FILE_NOT_FOUND) {
            throw std::runtime_error("RegDeleteKeyValueW failed: " + std::to_string(r));
        }
        return;
    }
    auto command = expectedCommand();
    if (command.empty()) {
        throw std::runtime_error("unable to determine current executable path");
    }
    auto r = RegSetKeyValueW(HKEY_CURRENT_USER, RUN_KEY, VALUE_NAME, REG_SZ, command.c_str(),
                             static_cast<DWORD>((command.size() + 1) * sizeof(wchar_t)));
    if (r != ERROR_SUCCESS) {
        throw std::runtime_error("RegSetKeyValueW failed: " + std::to_string(r));
    }
}
