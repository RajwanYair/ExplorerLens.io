#pragma once
// Error Recovery Engine — crash recovery, checkpoint save/restore
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// Recovery strategy
enum class RecoveryStrategy : uint32_t {
    Retry       = 0,
    Checkpoint  = 1,
    Rollback    = 2,
    SafeMode    = 3,
    FullReset   = 4,
    COUNT       = 5
};

/// Recovery state
enum class RecoveryState : uint32_t {
    Normal     = 0,
    Recovering = 1,
    Recovered  = 2,
    Failed     = 3,
    COUNT      = 4
};

/// Checkpoint data for recovery
struct Checkpoint {
    std::wstring  name;
    uint64_t      timestamp = 0;
    uint32_t      sequenceId = 0;
    std::wstring  state;
    bool          valid = true;
};

/// Manages crash recovery and checkpoint-based state restoration
class ErrorRecoveryEngine {
public:
    ErrorRecoveryEngine();

    static const wchar_t* GetStrategyName(RecoveryStrategy strategy);
    static const wchar_t* GetStateName(RecoveryState state);
    static uint32_t GetStrategyCount() { return static_cast<uint32_t>(RecoveryStrategy::COUNT); }

    /// Create a checkpoint
    uint32_t CreateCheckpoint(const std::wstring& name, const std::wstring& state);
    /// Restore from a checkpoint
    bool RestoreCheckpoint(uint32_t id);
    /// Get current state
    RecoveryState GetState() const { return m_state; }
    /// Get checkpoint count
    size_t GetCheckpointCount() const { return m_checkpoints.size(); }
    /// Get latest checkpoint
    const Checkpoint& GetLatestCheckpoint() const;
    /// Simulate a crash recovery
    bool RecoverFromCrash(RecoveryStrategy strategy);

private:
    RecoveryState m_state = RecoveryState::Normal;
    std::vector<Checkpoint> m_checkpoints;
    uint32_t m_nextSeqId = 1;
    static Checkpoint s_emptyCheckpoint;
};

}} // namespace ExplorerLens::Engine

