/**
 * PluginManager.h - ExplorerLens Plugin Management System
 * Version: 1.0.0
 *
 * Copyright (c) 2026 ExplorerLens Project
 *
 * Manages plugin lifecycle: discovery, loading, validation, execution, and
 * unloading. Enforces capability restrictions and compatibility requirements.
 */

#pragma once

#include "ExplorerLensPlugin.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

namespace ExplorerLens {
namespace PluginHost {

/**
 * Plugin State
 */
enum class PluginState {
  Discovered,   ///< Found but not loaded
  Loading,      ///< Currently loading
  Loaded,       ///< DLL loaded, not initialized
  Initializing, ///< Calling DT_Initialize()
  Active,       ///< Ready to generate thumbnails
  Disabled,     ///< Disabled by user or policy
  Failed,       ///< Failed to load or initialize
  Unloading     ///< Currently unloading
};

/**
 * Plugin Load Result
 */
enum class PluginLoadResult {
  Success,
  FileNotFound,
  InvalidPackage,
  InvalidManifest,
  AbiMismatch,
  EngineVersionMismatch,
  MissingExports,
  InitializationFailed,
  CapabilityDenied,
  SignatureInvalid,
  AlreadyLoaded,
  DependencyMissing
};

/**
 * Plugin Manifest (parsed from manifest.json)
 */
struct PluginManifest {
  uint32_t manifestVersion;
  std::wstring id;
  std::wstring name;
  std::wstring version;
  std::wstring vendor;
  std::wstring description;
  std::wstring homepage;
  uint32_t abiVersion;
  std::wstring minEngineVersion;
  std::wstring maxEngineVersion;
  std::vector<std::wstring> capabilities;
  std::wstring binaryPath;
  std::wstring binarySha256;
  size_t binarySize;
};

/**
 * Loaded Plugin Instance
 */
class LoadedPlugin {
public:
  LoadedPlugin(const std::wstring &packagePath, const PluginManifest &manifest);
  ~LoadedPlugin();

  // Lifecycle
  PluginLoadResult Load();
  PluginLoadResult Initialize();
  PluginLoadResult Shutdown();
  void Unload();

  // Execution
  DT_Status GenerateThumbnail(const DT_ThumbnailRequest *request,
                              DT_ThumbnailResult *result);
  DT_Status CanHandle(const wchar_t *filePath, IStream *stream,
                      uint32_t *confidence);

  // Metadata
  const PluginManifest &GetManifest() const { return m_manifest; }
  const DT_PluginInfo *GetInfo() const { return m_info; }
  PluginState GetState() const { return m_state; }
  const std::wstring &GetLastError() const { return m_lastError; }

  // Statistics
  const DT_PluginStatistics &GetStatistics() const { return m_statistics; }
  void UpdateStatistics(bool success, uint64_t elapsedUs);

  // Capability checking
  bool HasCapability(DT_Capability capability) const;
  bool IsCapabilityAllowed(DT_Capability capability) const;

private:
  std::wstring m_packagePath;
  PluginManifest m_manifest;
  HMODULE m_hModule;
  PluginState m_state;
  std::wstring m_lastError;

  // Function pointers
  const DT_PluginInfo *m_info;
  decltype(&DT_GetPluginInfo) m_fnGetInfo;
  decltype(&DT_GetSupportedFormats) m_fnGetFormats;
  decltype(&DT_Initialize) m_fnInitialize;
  decltype(&DT_Shutdown) m_fnShutdown;
  decltype(&DT_GenerateThumbnail) m_fnGenerateThumbnail;
  decltype(&DT_CanHandle) m_fnCanHandle;
  decltype(&DT_GetStatistics) m_fnGetStatistics;

  // Statistics
  DT_PluginStatistics m_statistics;

  // Helper methods
  bool LoadDll();
  bool LoadExports();
  bool ValidateExports();
  bool ValidateInfo();
  bool ValidateCompatibility();
  void SetError(const std::wstring &error);
};

/**
 * Plugin Manager
 * Central controller for all plugin operations.
 */
class PluginManager {
public:
  static PluginManager &Instance();

  // Lifecycle
  bool Initialize();
  void Shutdown();

  // Discovery
  size_t DiscoverPlugins(const std::wstring &pluginsDirectory);
  std::vector<PluginManifest> GetDiscoveredPlugins() const;

  // Loading
  PluginLoadResult LoadPlugin(const std::wstring &pluginId);
  PluginLoadResult LoadPluginFromPackage(const std::wstring &packagePath);
  void UnloadPlugin(const std::wstring &pluginId);
  void UnloadAllPlugins();

  // Management
  void EnablePlugin(const std::wstring &pluginId);
  void DisablePlugin(const std::wstring &pluginId);
  bool IsPluginLoaded(const std::wstring &pluginId) const;
  bool IsPluginEnabled(const std::wstring &pluginId) const;

  // Query
  LoadedPlugin *GetPlugin(const std::wstring &pluginId);
  std::vector<LoadedPlugin *>
  GetPluginsForExtension(const std::wstring &extension);
  std::vector<LoadedPlugin *> GetAllActivePlugins();

  // Execution
  DT_Status GenerateThumbnail(const std::wstring &pluginId,
                              const DT_ThumbnailRequest *request,
                              DT_ThumbnailResult *result);

  // Statistics
  struct ManagerStatistics {
    size_t totalPlugins;
    size_t activePlugins;
    size_t disabledPlugins;
    size_t failedPlugins;
    uint64_t totalRequests;
    uint64_t successfulRequests;
    uint64_t failedRequests;
  };
  ManagerStatistics GetStatistics() const;

  // Configuration
  void SetPluginsDirectory(const std::wstring &path);
  void SetCapabilityFilter(
      std::function<bool(const std::wstring &, DT_Capability)> filter);
  void SetMaxPluginInitTime(uint32_t milliseconds);
  void SetMaxPluginMemory(uint64_t bytes);

  // Capability enforcement
  bool IsCapabilityAllowed(const std::wstring &pluginId,
                           DT_Capability capability) const;
  void DenyCapability(DT_Capability capability);
  void AllowCapability(DT_Capability capability);

private:
  PluginManager();
  ~PluginManager();
  PluginManager(const PluginManager &) = delete;
  PluginManager &operator=(const PluginManager &) = delete;

  // Internal state
  bool m_initialized;
  std::wstring m_pluginsDirectory;
  std::unordered_map<std::wstring, std::unique_ptr<LoadedPlugin>> m_plugins;
  std::unordered_map<std::wstring, PluginManifest> m_discoveredPlugins;
  std::unordered_map<std::wstring, std::vector<std::wstring>> m_extensionMap;
  uint32_t m_deniedCapabilities;
  std::function<bool(const std::wstring &, DT_Capability)> m_capabilityFilter;
  uint32_t m_maxInitTimeMs;
  uint64_t m_maxPluginMemory;
  ManagerStatistics m_statistics;

  // Helper methods
  PluginManifest ParseManifest(const std::wstring &manifestPath);
  bool ValidateManifest(const PluginManifest &manifest);
  bool ValidatePackage(const std::wstring &packagePath);
  bool VerifyBinaryHash(const std::wstring &binaryPath,
                        const std::wstring &expectedSha256);
  bool CheckEngineCompatibility(const std::wstring &minVersion,
                                const std::wstring &maxVersion);
  void UpdateExtensionMap(const std::wstring &pluginId,
                          const DT_FormatInfo *formats, uint32_t count);
  void RemoveFromExtensionMap(const std::wstring &pluginId);
  std::wstring ExtractPackage(const std::wstring &packagePath);
};

/**
 * Plugin Sandbox Configuration
 */
struct SandboxConfig {
  enum class IsolationMode {
    InWorker,        ///< Run in same process (trusted plugins only)
    SeparateProcess, ///< Run in separate PluginHost.exe process (default)
    AppContainer     ///< Run in AppContainer (future, strongest isolation)
  };

  IsolationMode mode = IsolationMode::SeparateProcess;
  bool restrictedToken = true;
  bool jobObjectLimits = true;
  uint64_t maxMemoryBytes = 512 * 1024 * 1024; // 512 MB
  uint32_t maxCpuPercent = 80;
  uint32_t maxThreads = 16;
  bool allowNetworkAccess = false;
  bool allowFileSystemWrite = false;
  bool allowRegistryWrite = false;
};

/**
 * Plugin Host Process Manager
 * Manages out-of-process plugin execution 
 */
class PluginHostManager {
public:
  static PluginHostManager &Instance();

  bool Initialize();
  void Shutdown();

  // Process management
  bool LaunchPluginHost(const std::wstring &pluginId,
                        const SandboxConfig &config);
  bool TerminatePluginHost(const std::wstring &pluginId);
  bool IsPluginHostRunning(const std::wstring &pluginId);

  // IPC
  DT_Status SendRequest(const std::wstring &pluginId,
                        const DT_ThumbnailRequest *request,
                        DT_ThumbnailResult *result, uint32_t timeoutMs);

private:
  PluginHostManager();
  ~PluginHostManager();

  struct PluginHostProcess {
    HANDLE hProcess;
    HANDLE hThread;
    HANDLE hJobObject;
    HANDLE hPipeRead;
    HANDLE hPipeWrite;
    uint32_t processId;
    SandboxConfig config;
  };

  std::unordered_map<std::wstring, PluginHostProcess> m_processes;

  bool CreateRestrictedToken(HANDLE *outToken);
  bool CreateJobObject(HANDLE *outJobObject, const SandboxConfig &config);
  bool CreatePipes(HANDLE *hReadPipe, HANDLE *hWritePipe);
};

} // namespace PluginHost
} // namespace ExplorerLens

