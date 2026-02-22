# Sprint 312: Plugin Hot Reload

**Status:** ✅ Complete
**Component:** `Engine/Plugin/PluginHotReload.h`
**Tests:** 5 (TestPluginHotReload_TriggerNames, TestPluginHotReload_StateNames, TestPluginHotReload_PolicyNames, TestPluginHotReload_TriggerCount, TestPluginHotReload_StateCount)

## Overview
Live plugin DLL hot-reload allowing developers to iterate on plugin code without restarting the shell extension host process.

## Key Features
- HotReloadTrigger: Manual, FileChange, VersionBump, ScheduleInterval, SignalUSR1
- HotReloadState: Idle, Detecting, Unloading, Loading, Verifying, Active, Failed
- HotReloadPolicy: AlwaysReload, OnlyIfNewer, RequireSignature, DevModeOnly
- Shadow-copy DLL pattern avoids file-lock issues on Windows
