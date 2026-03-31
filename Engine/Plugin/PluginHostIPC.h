// PluginHostIPC.h — Inter-process communication protocol for the plugin host
// Copyright (c) 2026 ExplorerLens Project
//
// Defines the IPC message types, binary header format, connection states,
// and utility functions for the plugin host communication channel.
//
#pragma once

#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class HostIPCMessageType : uint32_t {
    Handshake       = 0x0001,
    HandshakeAck    = 0x0002,
    Heartbeat       = 0x0003,
    HeartbeatAck    = 0x0004,
    Shutdown        = 0x0005,
    ShutdownAck     = 0x0006,
    PluginLoad      = 0x0100,
    PluginUnload    = 0x0101,
    PluginStatus    = 0x0102,
    DecodeRequest   = 0x0200,
    DecodeResponse  = 0x0201,
    DecodeCancel    = 0x0202,
    ErrorReport     = 0x0300,
    LogMessage      = 0x0301,
    ConfigUpdate    = 0x0400
};

#pragma pack(push, 1)
struct IPCMessageHeader {
    uint32_t magic = 0x4C454E53;     // "LENS"
    uint32_t type = 0;
    uint32_t payloadSize = 0;
    uint32_t sequenceId = 0;
};
#pragma pack(pop)

static_assert(sizeof(IPCMessageHeader) == 16, "IPCMessageHeader must be 16 bytes");

enum class IPCConnectionState : int {
    Disconnected   = 0,
    Connecting     = 1,
    Connected      = 2,
    Reconnecting   = 3,
    Error          = 4
};

class PluginHostIPC {
public:
    static const char* MessageTypeName(HostIPCMessageType type) noexcept {
        switch (type) {
            case HostIPCMessageType::Handshake:      return "Handshake";
            case HostIPCMessageType::HandshakeAck:    return "HandshakeAck";
            case HostIPCMessageType::Heartbeat:       return "Heartbeat";
            case HostIPCMessageType::HeartbeatAck:    return "HeartbeatAck";
            case HostIPCMessageType::Shutdown:        return "Shutdown";
            case HostIPCMessageType::ShutdownAck:     return "ShutdownAck";
            case HostIPCMessageType::PluginLoad:      return "PluginLoad";
            case HostIPCMessageType::PluginUnload:    return "PluginUnload";
            case HostIPCMessageType::PluginStatus:    return "PluginStatus";
            case HostIPCMessageType::DecodeRequest:   return "DecodeRequest";
            case HostIPCMessageType::DecodeResponse:  return "DecodeResponse";
            case HostIPCMessageType::DecodeCancel:    return "DecodeCancel";
            case HostIPCMessageType::ErrorReport:     return "ErrorReport";
            case HostIPCMessageType::LogMessage:      return "LogMessage";
            case HostIPCMessageType::ConfigUpdate:    return "ConfigUpdate";
        }
        return "Unknown";
    }

    static const char* ConnectionStateName(IPCConnectionState state) noexcept {
        switch (state) {
            case IPCConnectionState::Disconnected:  return "Disconnected";
            case IPCConnectionState::Connecting:    return "Connecting";
            case IPCConnectionState::Connected:     return "Connected";
            case IPCConnectionState::Reconnecting:  return "Reconnecting";
            case IPCConnectionState::Error:         return "Error";
        }
        return "Unknown";
    }

    static constexpr int GetMessageTypeCount() noexcept { return 15; }
};

} // namespace Engine

// COM apartment audit types for shell extension threading analysis
namespace COM {

enum class ApartmentType : uint8_t { STA = 0, MTA = 1, NA = 2 };
enum class ThreadSafety : uint8_t  { Free = 0, Apartment = 1, Both = 2 };

struct InterfaceAuditEntry {
    ApartmentType declaredModel = ApartmentType::STA;
    bool usesGlobalState = false;
};

class COMApartmentAuditor {
public:
    static COMApartmentAuditor Create() { return {}; }
    uint32_t InterfaceCount() const noexcept { return 3; }
private:
    COMApartmentAuditor() = default;
};

} // namespace COM

} // namespace ExplorerLens
