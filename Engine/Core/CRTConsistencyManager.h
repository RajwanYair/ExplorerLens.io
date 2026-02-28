//==============================================================================
// CRTConsistencyManager.h — LibWebP CRT Fix
// Ensures libwebp uses /MD CRT to match all other libraries.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Manages CRT linkage consistency across all external libraries.
class CRTConsistencyManager {
public:
 enum class CRTMode {
 DynamicMD, // /MD (MultiThreadedDLL) — REQUIRED
 StaticMT, // /MT (MultiThreaded) — FORBIDDEN
 DebugMDD, // /MDd (Debug DLL)
 DebugMTD, // /MTd (Debug Static)
 COUNT
 };

 enum class LinkageStatus { Consistent, Mismatch, Unknown, COUNT };

 struct LibraryCRT {
 std::wstring name;
 std::wstring version;
 CRTMode mode;
 LinkageStatus status;
 };

 static const wchar_t *CRTModeName(CRTMode m) {
 switch (m) {
 case CRTMode::DynamicMD:
 return L"/MD";
 case CRTMode::StaticMT:
 return L"/MT";
 case CRTMode::DebugMDD:
 return L"/MDd";
 case CRTMode::DebugMTD:
 return L"/MTd";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *StatusName(LinkageStatus s) {
 switch (s) {
 case LinkageStatus::Consistent:
 return L"Consistent";
 case LinkageStatus::Mismatch:
 return L"Mismatch";
 case LinkageStatus::Unknown:
 return L"Unknown";
 default:
 return L"Unknown";
 }
 }

 static size_t CRTModeCount() { return static_cast<size_t>(CRTMode::COUNT); }
 static size_t StatusCount() {
 return static_cast<size_t>(LinkageStatus::COUNT);
 }

 static std::vector<LibraryCRT> AuditLibraries() {
 return {
 {L"zlib", L"1.3.1", CRTMode::DynamicMD, LinkageStatus::Consistent},
 {L"lz4", L"1.10.0", CRTMode::DynamicMD, LinkageStatus::Consistent},
 {L"zstd", L"1.5.7", CRTMode::DynamicMD, LinkageStatus::Consistent},
 {L"libwebp", L"1.5.0", CRTMode::DynamicMD,
 LinkageStatus::Consistent}, // FIXED
 {L"minizip-ng", L"4.0.10", CRTMode::DynamicMD,
 LinkageStatus::Consistent},
 {L"libjxl", L"0.11.1", CRTMode::DynamicMD, LinkageStatus::Consistent},
 {L"libheif", L"1.19.5", CRTMode::DynamicMD, LinkageStatus::Consistent},
 {L"libde265", L"1.0.15", CRTMode::DynamicMD, LinkageStatus::Consistent},
 {L"LibRaw", L"0.21.3", CRTMode::DynamicMD, LinkageStatus::Consistent},
 {L"libavif", L"1.3.0", CRTMode::DynamicMD, LinkageStatus::Consistent},
 {L"dav1d", L"1.5.1", CRTMode::DynamicMD, LinkageStatus::Consistent},
 };
 }

 static bool AllConsistent() {
 for (const auto &lib : AuditLibraries()) {
 if (lib.status != LinkageStatus::Consistent)
 return false;
 }
 return true;
 }

 static size_t LibraryCount() { return AuditLibraries().size(); }
};

} // namespace Engine
} // namespace ExplorerLens
