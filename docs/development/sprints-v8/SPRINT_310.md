# Sprint 310: Plugin SDK V2

**Status:** ✅ Complete
**Component:** `Engine/Plugin/PluginSDKV2.h`
**Tests:** 5 (TestPluginSDKV2_CapabilityNames, TestPluginSDKV2_LifecycleNames, TestPluginSDKV2_APIVersionNames, TestPluginSDKV2_CapCount, TestPluginSDKV2_LifecycleCount)

## Overview
Extended plugin API surface adding host-side services (logging, telemetry, settings, cache, GPU), async decode callbacks, and plugin marketplace metadata.

## Key Features
- PluginSDKV2Capability: Decode, Encode, Transform, Cache, GPU, Logging, Telemetry, Settings, Marketplace (9 capabilities)
- PluginSDKV2LifeCycle: Unloaded, Loading, Loaded, Active, Suspended, Unloading, Error
- PluginAPIVersion: V1_0, V1_5, V2_0, V2_1
- Host service injection via IPluginHostServices V2 interface
