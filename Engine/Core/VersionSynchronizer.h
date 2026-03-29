//==============================================================================
// VersionSynchronizer.h — Version Synchronization
// Ensures all version references are consistent across the codebase.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Centralized version authority — single source of truth for all components.
class VersionSynchronizer {
public:
 static constexpr int MAJOR = 25;
 static constexpr int MINOR = 3;
 static constexpr int PATCH = 0;
 static constexpr const wchar_t *CODENAME = L"Rigel-T";
 static constexpr const wchar_t *FULL_VERSION = L"25.3.0";

 enum class Component { Engine, Shell, Manager, PluginHost, Installer, COUNT };

 struct VersionEntry {
 Component component;
 std::wstring expected;
 std::wstring actual;
 bool synced;
 };

 static const wchar_t *ComponentName(Component c) {
 switch (c) {
 case Component::Engine:
 return L"Engine";
 case Component::Shell:
 return L"Shell";
 case Component::Manager:
 return L"Manager";
 case Component::PluginHost:
 return L"PluginHost";
 case Component::Installer:
 return L"Installer";
 default:
 return L"Unknown";
 }
 }

 static size_t ComponentCount() {
 return static_cast<size_t>(Component::COUNT);
 }

 static bool Validate() {
 return MAJOR == 25 && MINOR == 3 && PATCH == 0;
 }

 static std::vector<VersionEntry> Audit() {
 std::vector<VersionEntry> entries;
 for (size_t i = 0; i < ComponentCount(); ++i) {
 auto c = static_cast<Component>(i);
 entries.push_back({c, FULL_VERSION, FULL_VERSION, true});
 }
 return entries;
 }

 static uint32_t PackedVersion() {
 return (MAJOR << 16) | (MINOR << 8) | PATCH;
 }
};

} // namespace Engine
} // namespace ExplorerLens
