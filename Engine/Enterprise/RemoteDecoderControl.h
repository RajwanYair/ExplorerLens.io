// RemoteDecoderControl.h — Remote Decoder Enable/Disable/Quarantine Control
// Copyright (c) 2026 ExplorerLens Project
//
// Provides RemoteDecoderControl for fleet-wide decoder state management including
// remote enable, disable, quarantine, restore, and configuration broadcast operations.
//
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <cstdint>
#include <optional>

namespace ExplorerLens::Engine {

enum class DecoderAction : uint8_t {
    Enable       = 0,
    Disable      = 1,
    Quarantine   = 2,
    Restore      = 3,
    UpdateConfig = 4
};

enum class DecoderState : uint8_t {
    Active      = 0,
    Disabled    = 1,
    Quarantined = 2,
    Unknown     = 3
};

struct DecoderTarget {
    std::string  decoderId;
    uint32_t     endpointCount{0};
    DecoderState currentState{DecoderState::Unknown};
    std::string  version;
    std::chrono::system_clock::time_point lastContactAt;
};

struct ControlResult {
    bool                     success{false};
    uint32_t                 affected{0};
    std::vector<std::string> errors;
    uint32_t                 durationMs{0};
};

class RemoteDecoderControl {
public:
    RemoteDecoderControl()  = default;
    ~RemoteDecoderControl() = default;

    RemoteDecoderControl(const RemoteDecoderControl&)            = delete;
    RemoteDecoderControl& operator=(const RemoteDecoderControl&) = delete;

    // Core control operations
    ControlResult Execute(const std::string& decoderId, DecoderAction action);
    ControlResult Execute(const std::string& decoderId, DecoderAction action,
                          const std::string& configJson);

    // State queries
    std::optional<DecoderTarget> GetDecoderState(const std::string& decoderId) const;
    std::vector<DecoderTarget>   ListDecoders() const;
    std::vector<DecoderTarget>   GetQuarantinedDecoders() const;
    std::vector<DecoderTarget>   GetDisabledDecoders() const;

    // Convenience wrappers
    ControlResult QuarantineDecoder(const std::string& decoderId,
                                    const std::string& reason);
    ControlResult RestoreDecoder(const std::string& decoderId);

    // Fleet-wide broadcast
    ControlResult BroadcastAction(DecoderAction action,
                                  const std::vector<std::string>& decoderIds);

    // Register/unregister decoders for remote control
    bool RegisterDecoder(DecoderTarget target);
    bool UnregisterDecoder(const std::string& decoderId);

    // Action callback
    using ActionCallback = std::function<void(const std::string&, const ControlResult&)>;
    void SetActionCallback(ActionCallback cb);

private:
    std::vector<DecoderTarget> m_decoders;
    ActionCallback             m_actionCallback;

    bool          ValidateDecoderId(const std::string& id) const;
    ControlResult ApplyAction(DecoderTarget& target, DecoderAction action,
                               const std::string& configJson);
};

} // namespace ExplorerLens::Engine
