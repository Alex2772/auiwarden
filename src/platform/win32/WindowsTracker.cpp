#include "WindowsTracker.h"

#include <chrono>
#include <Windows.h>

using namespace std::chrono_literals;

static constexpr auto TIMEOUT = 2min;

void WindowsTracker::getCurrentActivity(Activity& activity) {
    HWND window = GetForegroundWindow();
    if (!window)
        return;

    WCHAR wbuf[512] {};
    int wbuf_len = GetWindowTextW(window, wbuf, std::size(wbuf));
    if (!wbuf_len)
        return;

    CHAR ubuf[1024] {};
    int ubuf_len = WideCharToMultiByte(CP_UTF8, 0, wbuf, wbuf_len, ubuf, std::size(ubuf), nullptr, nullptr);
    if (!ubuf_len)
        return;

    activity.activeWindowTitle = ubuf;

    LASTINPUTINFO info { .cbSize = sizeof(info) };
    if (!GetLastInputInfo(&info))
        return;

    auto elapsed_time = std::chrono::milliseconds(GetTickCount() - info.dwTime);
    if (elapsed_time >= TIMEOUT) {
        activity.idle = Idle::AWAY_FROM_KEYBOARD;
    } else {
        activity.idle = Idle::USER_PRESENT;
    }
}
