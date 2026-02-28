// FormatStatusIndicator.h — Traffic-Light Status Indicator for Format Decoders
// Copyright (c) 2026 ExplorerLens Project
//
// Provides visual status indicators for each format decoder:
// Green = fully operational, decoder + library loaded
// Yellow = available but degraded (fallback decoder, missing GPU accel)
// Red = unavailable (library not built, decoder disabled)
// Gray = not applicable (format not registered)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// High-level format availability status
enum class FormatStatus : uint8_t {
 Active = 0, ///< Fully operational
 Degraded = 1, ///< Available but limited
 Unavailable = 2, ///< Not usable
 Unknown = 3, ///< Not registered
 COUNT
};

/// Decoder health status
enum class DecoderStatus : uint8_t {
 Green = 0, ///< Fully operational
 Yellow = 1, ///< Degraded (fallback active)
 Red = 2, ///< Unavailable / broken
 Gray = 3 ///< Not registered
};

/// Reason for non-green status
enum class StatusReason : uint16_t {
 None = 0,
 LibraryNotBuilt = 1, ///< External library not compiled
 LibraryLoadFailed = 2, ///< DLL/static lib failed to load
 DecoderDisabled = 3, ///< User disabled in settings
 GPUNotAvailable = 4, ///< GPU accel unavailable, CPU fallback
 FallbackActive = 5, ///< Using WIC/GDI+ fallback
 PartialSupport = 6, ///< Some features missing
 LicenseRestriction = 7, ///< License limits usage
 NotRegistered = 8, ///< Format not in registry
 CRTMismatch = 9, ///< /MT vs /MD conflict (e.g., libwebp)
 VersionMismatch = 10, ///< Library version incompatible
};

/// Status report for a single decoder
struct DecoderStatusEntry {
 const char *formatName = nullptr; ///< e.g., "WebP"
 const char *decoderClass = nullptr; ///< e.g., "WebPDecoder"
 const char *libraryName = nullptr; ///< e.g., "libwebp 1.5.0"
 DecoderStatus status = DecoderStatus::Gray;
 StatusReason reason = StatusReason::None;
 const char *tooltip = nullptr; ///< Human-readable explanation
 uint32_t avgDecodeTimeUs = 0; ///< Average decode time in microseconds
 uint32_t totalDecodes = 0; ///< Lifetime decode count
 uint32_t failureCount = 0; ///< Lifetime failure count
};

/// Traffic-light color constants (RGB)
struct StatusColors {
 static constexpr COLORREF Green = RGB(0, 180, 0);
 static constexpr COLORREF Yellow = RGB(220, 180, 0);
 static constexpr COLORREF Red = RGB(220, 50, 50);
 static constexpr COLORREF Gray = RGB(160, 160, 160);

 /// Dark mode variants
 static constexpr COLORREF DarkGreen = RGB(50, 200, 80);
 static constexpr COLORREF DarkYellow = RGB(240, 200, 40);
 static constexpr COLORREF DarkRed = RGB(240, 80, 80);
 static constexpr COLORREF DarkGray = RGB(100, 100, 100);

 static COLORREF GetColor(DecoderStatus s, bool darkMode = false) {
 if (darkMode) {
 switch (s) {
 case DecoderStatus::Green:
 return DarkGreen;
 case DecoderStatus::Yellow:
 return DarkYellow;
 case DecoderStatus::Red:
 return DarkRed;
 default:
 return DarkGray;
 }
 }
 switch (s) {
 case DecoderStatus::Green:
 return Green;
 case DecoderStatus::Yellow:
 return Yellow;
 case DecoderStatus::Red:
 return Red;
 default:
 return Gray;
 }
 }
};

/// Status indicator manager
class FormatStatusIndicator {
public:
 static FormatStatusIndicator &Instance() {
 static FormatStatusIndicator inst;
 return inst;
 }

 /// Add or update a decoder status entry
 void ReportStatus(const char *formatName, const char *decoderClass,
 const char *libName, DecoderStatus status,
 StatusReason reason = StatusReason::None,
 const char *tooltip = nullptr) {
 for (auto &e : m_entries) {
 if (e.formatName && formatName && strcmp(e.formatName, formatName) == 0) {
 e.decoderClass = decoderClass;
 e.libraryName = libName;
 e.status = status;
 e.reason = reason;
 e.tooltip = tooltip;
 return;
 }
 }
 m_entries.push_back(
 {formatName, decoderClass, libName, status, reason, tooltip, 0, 0, 0});
 }

 /// Record a decode event for statistics
 void RecordDecode(const char *formatName, uint32_t timeUs, bool success) {
 for (auto &e : m_entries) {
 if (e.formatName && formatName && strcmp(e.formatName, formatName) == 0) {
 e.totalDecodes++;
 if (!success)
 e.failureCount++;
 // Running average
 if (e.avgDecodeTimeUs == 0)
 e.avgDecodeTimeUs = timeUs;
 else
 e.avgDecodeTimeUs = (e.avgDecodeTimeUs * 3 + timeUs) / 4;
 return;
 }
 }
 }

 /// Get all entries
 const std::vector<DecoderStatusEntry> &GetEntries() const {
 return m_entries;
 }

 /// Count by status
 uint32_t CountByStatus(DecoderStatus s) const {
 uint32_t c = 0;
 for (const auto &e : m_entries)
 if (e.status == s)
 ++c;
 return c;
 }

 /// Overall health percentage (green = 100%, yellow = 50%, red/gray = 0%)
 float GetHealthPercent() const {
 if (m_entries.empty())
 return 100.0f;
 float sum = 0;
 for (const auto &e : m_entries) {
 switch (e.status) {
 case DecoderStatus::Green:
 sum += 1.0f;
 break;
 case DecoderStatus::Yellow:
 sum += 0.5f;
 break;
 default:
 break;
 }
 }
 return (sum / static_cast<float>(m_entries.size())) * 100.0f;
 }

 /// Render a small status circle (owner-draw helper)
 static void DrawStatusCircle(HDC hdc, int x, int y, int radius,
 DecoderStatus status, bool darkMode = false) {
 COLORREF color = StatusColors::GetColor(status, darkMode);
 HBRUSH brush = CreateSolidBrush(color);
 HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
 HPEN pen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
 HPEN oldPen = (HPEN)SelectObject(hdc, pen);
 Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
 SelectObject(hdc, oldPen);
 SelectObject(hdc, oldBrush);
 DeleteObject(pen);
 DeleteObject(brush);
 }

 /// Reason name for tooltip generation
 static const char *ReasonName(StatusReason r) {
 switch (r) {
 case StatusReason::None:
 return "OK";
 case StatusReason::LibraryNotBuilt:
 return "Library not built";
 case StatusReason::LibraryLoadFailed:
 return "Library load failed";
 case StatusReason::DecoderDisabled:
 return "Disabled by user";
 case StatusReason::GPUNotAvailable:
 return "GPU unavailable, using CPU";
 case StatusReason::FallbackActive:
 return "Using fallback decoder";
 case StatusReason::PartialSupport:
 return "Partial support";
 case StatusReason::LicenseRestriction:
 return "License restriction";
 case StatusReason::NotRegistered:
 return "Not registered";
 case StatusReason::CRTMismatch:
 return "CRT linkage mismatch";
 case StatusReason::VersionMismatch:
 return "Version mismatch";
 default:
 return "Unknown";
 }
 }

 /// Format-level status queries
 static constexpr size_t StatusCount() {
 return static_cast<size_t>(FormatStatus::COUNT);
 }

 static const wchar_t *StatusName(FormatStatus s) {
 switch (s) {
 case FormatStatus::Active:
 return L"Active";
 case FormatStatus::Degraded:
 return L"Degraded";
 case FormatStatus::Unavailable:
 return L"Unavailable";
 case FormatStatus::Unknown:
 return L"Unknown";
 default:
 return L"Unknown";
 }
 }

private:
 FormatStatusIndicator() = default;
 std::vector<DecoderStatusEntry> m_entries;
};

} // namespace Engine
} // namespace ExplorerLens
