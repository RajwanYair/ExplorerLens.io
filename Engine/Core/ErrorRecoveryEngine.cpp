#include "ErrorRecoveryEngine.h"
#include <chrono>

namespace ExplorerLens { namespace Engine {

Checkpoint ErrorRecoveryEngine::s_emptyCheckpoint = {};

ErrorRecoveryEngine::ErrorRecoveryEngine() = default;

const wchar_t* ErrorRecoveryEngine::GetStrategyName(RecoveryStrategy strategy) {
    switch (strategy) {
        case RecoveryStrategy::Retry:      return L"Retry";
        case RecoveryStrategy::Checkpoint: return L"Checkpoint";
        case RecoveryStrategy::Rollback:   return L"Rollback";
        case RecoveryStrategy::SafeMode:   return L"Safe Mode";
        case RecoveryStrategy::FullReset:  return L"Full Reset";
        default:                           return L"Unknown";
    }
}

const wchar_t* ErrorRecoveryEngine::GetStateName(RecoveryState state) {
    switch (state) {
        case RecoveryState::Normal:     return L"Normal";
        case RecoveryState::Recovering: return L"Recovering";
        case RecoveryState::Recovered:  return L"Recovered";
        case RecoveryState::Failed:     return L"Failed";
        default:                        return L"Unknown";
    }
}

uint32_t ErrorRecoveryEngine::CreateCheckpoint(const std::wstring& name, const std::wstring& state) {
    Checkpoint cp;
    cp.name = name;
    cp.state = state;
    cp.sequenceId = m_nextSeqId++;
    auto now = std::chrono::system_clock::now();
    cp.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    cp.valid = true;
    m_checkpoints.push_back(std::move(cp));
    return cp.sequenceId;
}

bool ErrorRecoveryEngine::RestoreCheckpoint(uint32_t id) {
    for (const auto& cp : m_checkpoints) {
        if (cp.sequenceId == id && cp.valid) {
            m_state = RecoveryState::Recovered;
            return true;
        }
    }
    return false;
}

const Checkpoint& ErrorRecoveryEngine::GetLatestCheckpoint() const {
    if (m_checkpoints.empty()) return s_emptyCheckpoint;
    return m_checkpoints.back();
}

bool ErrorRecoveryEngine::RecoverFromCrash(RecoveryStrategy strategy) {
    m_state = RecoveryState::Recovering;
    switch (strategy) {
        case RecoveryStrategy::Retry:
        case RecoveryStrategy::SafeMode:
        case RecoveryStrategy::FullReset:
            m_state = RecoveryState::Recovered;
            return true;
        case RecoveryStrategy::Checkpoint:
            if (!m_checkpoints.empty()) {
                m_state = RecoveryState::Recovered;
                return true;
            }
            m_state = RecoveryState::Failed;
            return false;
        case RecoveryStrategy::Rollback:
            if (m_checkpoints.size() > 1) {
                m_checkpoints.pop_back();
                m_state = RecoveryState::Recovered;
                return true;
            }
            m_state = RecoveryState::Failed;
            return false;
        default:
            m_state = RecoveryState::Failed;
            return false;
    }
}

}} // namespace ExplorerLens::Engine

