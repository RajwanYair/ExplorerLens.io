// PluginRuntimeValidation.h — Plugin lifecycle state validation and test scenarios
// Copyright (c) 2026 ExplorerLens Project
//
// Provides the plugin validation state machine, IPC transport selection,
// test scenario construction, and state transition validation.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Plugin {

enum class ValidationPluginState : int {
    Unloaded = 0,
    Discovering = 1,
    Loading = 2,
    Initializing = 3,
    Ready = 4,
    Running = 5,
    Stopping = 6,
    Faulted = 7
};

enum class IPCTransport : int {
    NamedPipe = 0,
    SharedMemory = 1
};

struct ValidationTestScenario
{
    std::string extension;
    bool expectSuccess = true;
    bool injectFault = false;
    uint32_t timeoutMs = 5000;

    static ValidationTestScenario NormalDecode(const std::string& ext)
    {
        ValidationTestScenario s;
        s.extension = ext;
        s.expectSuccess = true;
        s.injectFault = false;
        return s;
    }

    static ValidationTestScenario CrashInjection()
    {
        ValidationTestScenario s;
        s.extension = ".crash";
        s.expectSuccess = false;
        s.injectFault = true;
        return s;
    }
};

// PluginRuntimeValidator is defined in PluginSecurity.h
// (IsValidTransition is a template method on that class)

}  // namespace Plugin
}  // namespace ExplorerLens
