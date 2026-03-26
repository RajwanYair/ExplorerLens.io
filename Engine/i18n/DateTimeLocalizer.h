// DateTimeLocalizer.h — Locale-Aware File Date and Time Formatting
// Copyright (c) 2026 ExplorerLens Project
//
// Converts FILETIME and system timestamps to locale-appropriate display strings
// using Windows NLS GetDateFormatEx / GetTimeFormatEx. Supports relative
// date strings ("2 hours ago", "Yesterday") for recency indicators.
//
#pragma once

#include <windows.h>
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine { namespace i18n {

enum class DateStyle : uint8_t {
    Short    = 0,   // "3/26/2026" or locale equivalent
    Long     = 1,   // "March 26, 2026"
    Relative = 2,   // "2 hours ago", "Yesterday", "3 days ago"
    ISO8601  = 3    // "2026-03-26T14:22:01Z" (invariant)
};

enum class TimeStyle : uint8_t {
    None   = 0,
    Short  = 1,   // "2:22 PM"
    Long   = 2    // "2:22:01 PM"
};

class DateTimeLocalizer {
public:
    static DateTimeLocalizer& Instance() {
        static DateTimeLocalizer inst;
        return inst;
    }

    // Format a FILETIME for display in file info overlays
    std::wstring Format(const FILETIME& ft, DateStyle dateStyle = DateStyle::Short,
                         TimeStyle timeStyle = TimeStyle::Short) const {
        SYSTEMTIME st;
        FileTimeToLocalFileTime(&ft, const_cast<FILETIME*>(&ft));
        FileTimeToSystemTime(&ft, &st);
        return FormatST(st, dateStyle, timeStyle);
    }

    // Format a SYSTEMTIME directly
    std::wstring FormatST(const SYSTEMTIME& st, DateStyle dateStyle,
                           TimeStyle timeStyle) const {
        if (dateStyle == DateStyle::ISO8601) {
            wchar_t buf[32];
            swprintf_s(buf, 32, L"%04d-%02d-%02dT%02d:%02d:%02dZ",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
            return buf;
        }

        if (dateStyle == DateStyle::Relative) return RelativeDate(st);

        wchar_t dateBuf[64] = {}, timeBuf[64] = {};
        DWORD dateFlags = (dateStyle == DateStyle::Long) ? DATE_LONGDATE : DATE_SHORTDATE;
        GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, dateFlags, &st, nullptr, dateBuf, 64, nullptr);

        if (timeStyle == TimeStyle::None) return dateBuf;

        DWORD timeFlags = (timeStyle == TimeStyle::Long) ? 0 : TIME_NOSECONDS;
        GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, timeFlags, &st, nullptr, timeBuf, 64);

        return std::wstring(dateBuf) + L" " + timeBuf;
    }

    // "X minutes ago", "Yesterday", etc.
    std::wstring RelativeDate(const SYSTEMTIME& st) const {
        SYSTEMTIME now;
        GetLocalTime(&now);

        FILETIME ftNow, ftThen;
        SystemTimeToFileTime(&now, &ftNow);
        SystemTimeToFileTime(&st, &ftThen);

        ULARGE_INTEGER uNow, uThen;
        uNow.LowPart  = ftNow.dwLowDateTime;  uNow.HighPart  = ftNow.dwHighDateTime;
        uThen.LowPart = ftThen.dwLowDateTime; uThen.HighPart = ftThen.dwHighDateTime;

        int64_t diffSec = static_cast<int64_t>((uNow.QuadPart - uThen.QuadPart) / 10000000ULL);
        if (diffSec < 60)       return L"Just now";
        if (diffSec < 3600)     { wchar_t b[32]; swprintf_s(b,32,L"%lld minutes ago", diffSec/60); return b; }
        if (diffSec < 86400)    { wchar_t b[32]; swprintf_s(b,32,L"%lld hours ago",   diffSec/3600); return b; }
        if (diffSec < 172800)   return L"Yesterday";
        if (diffSec < 604800)   { wchar_t b[32]; swprintf_s(b,32,L"%lld days ago",    diffSec/86400); return b; }
        if (diffSec < 2592000)  { wchar_t b[32]; swprintf_s(b,32,L"%lld weeks ago",   diffSec/604800); return b; }

        // Fall back to short date
        return FormatST(st, DateStyle::Short, TimeStyle::None);
    }

    // Format a duration in milliseconds as "1.23 s" or "450 ms"
    std::wstring FormatDuration(float ms) const {
        wchar_t buf[32];
        if (ms < 1000.f) swprintf_s(buf, 32, L"%.0f ms", ms);
        else              swprintf_s(buf, 32, L"%.2f s", ms / 1000.f);
        return buf;
    }

private:
    DateTimeLocalizer() = default;
};

}}} // namespace ExplorerLens::Engine::i18n
