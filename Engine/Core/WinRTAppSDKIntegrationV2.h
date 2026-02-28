//==============================================================================
// ExplorerLens Engine — WinRT / App SDK Integration V2
// Windows App SDK 2.0 thumbnail provider contracts, WinRT thumbnail
// stream interface, ExternalLocation activation, and Bootstrap API
// lifecycle for packaged + unpackaged DLL scenarios.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class WinRTActivationKind : uint8_t { Unpackaged=0,Packaged,ExternalLocation,SparsePackage,COUNT };
enum class AppSDKBootstrapPhase : uint8_t { Initialize=0,Bootstrap,MainInstance,Unload,COUNT };
enum class WinRTStreamMode : uint8_t { Sync=0, Async, RandomAccess, COUNT };

struct WinRTThumbnailContract {
 std::wstring packageFamilyName;
 WinRTActivationKind activation = WinRTActivationKind::Unpackaged;
 WinRTStreamMode streamMode = WinRTStreamMode::Async;
 bool supportsHDR = false;
 bool supportsDPI = false;
 uint32_t maxThumbSizePx = 1024;
};

struct AppSDKBootstrapStatus {
 AppSDKBootstrapPhase phase = AppSDKBootstrapPhase::Initialize;
 uint32_t sdkMajorVer = 2;
 uint32_t sdkMinorVer = 0;
 bool ddlImportOK = false;
 bool bootstrapOK = false;
};

class WinRTAppSDKIntegrationV2 {
public:
 static const wchar_t* ActivationKindName(WinRTActivationKind k) {
 switch(k) {
 case WinRTActivationKind::Unpackaged: return L"Unpackaged";
 case WinRTActivationKind::Packaged: return L"Packaged";
 case WinRTActivationKind::ExternalLocation: return L"External Location";
 case WinRTActivationKind::SparsePackage: return L"Sparse Package";
 default: return L"Unknown";
 }
 }
 static const wchar_t* BootstrapPhaseName(AppSDKBootstrapPhase p) {
 switch(p) {
 case AppSDKBootstrapPhase::Initialize: return L"Initialize";
 case AppSDKBootstrapPhase::Bootstrap: return L"Bootstrap";
 case AppSDKBootstrapPhase::MainInstance: return L"Main Instance";
 case AppSDKBootstrapPhase::Unload: return L"Unload";
 default: return L"Unknown";
 }
 }
 static const wchar_t* StreamModeName(WinRTStreamMode m) {
 switch(m) {
 case WinRTStreamMode::Sync: return L"Synchronous";
 case WinRTStreamMode::Async: return L"Asynchronous";
 case WinRTStreamMode::RandomAccess: return L"Random Access";
 default: return L"Unknown";
 }
 }
 static constexpr size_t ActivationKindCount() { return static_cast<size_t>(WinRTActivationKind::COUNT); }
 static constexpr size_t BootstrapPhaseCount() { return static_cast<size_t>(AppSDKBootstrapPhase::COUNT); }
 static constexpr size_t StreamModeCount() { return static_cast<size_t>(WinRTStreamMode::COUNT); }
};

}} // namespace ExplorerLens::Engine

