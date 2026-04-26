// Engine/Core/CancelAwareBindCallback.h
// ExplorerLens — Cancel-aware IBindStatusCallback wrapper (H36 / ROADMAP v7.0 Phase 2)
// Sprint S313.
//
// Purpose:
//   The Windows Shell infrastructure calls IBindStatusCallback::OnProgress()
//   periodically during asynchronous bind/moniker operations.  By integrating
//   AsyncDecodeToken into this callback chain, we can propagate cooperative
//   cancellation from the shell's STA thread into the decode pipeline.
//
// Protocol:
//   1. Shell calls IBindStatusCallback::OnProgress() with bytes-read count.
//   2. CancelAwareBindCallback checks DecodeTokenView::IsCancellationRequested().
//   3. If cancellation requested → returns E_ABORT (BINDSTATUS_DOWNLOADINGDATA).
//   4. Shell propagates E_ABORT, decode pipeline returns DecodeResult::Cancelled.
//
// Usage (Phase 2):
//   AsyncDecodeToken tok;
//   CancelAwareBindCallback cb{ DecodeTokenView{tok} };
//   moniker->BindToStorage(&bc, &cb, ...);
//
// NOTE: This header is COM-interface–aware but does NOT include objidl.h.
//       The concrete CancelAwareBindCallback.cpp (Phase 2) will include
//       COM headers and implement QueryInterface / AddRef / Release.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_CANCEL_AWARE_BIND_CALLBACK_H
#define EXPLORERLENS_ENGINE_CANCEL_AWARE_BIND_CALLBACK_H

#include <cstdint>
#include <atomic>
#include "../Core/AsyncDecodeToken.h"

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// BindCallbackStatus — progress codes reported by OnProgress()
// ---------------------------------------------------------------------------
enum class BindCallbackStatus : std::uint32_t {
    FINDING_RESOURCE  = 1,
    CONNECTING        = 2,
    REDIRECTING       = 3,
    BEGINNING_DOWNLOAD= 4,
    DOWNLOADING       = 5,   // BINDSTATUS_DOWNLOADINGDATA
    ENDING_DOWNLOAD   = 6,
    CANCELLED         = 7,   // Returned after stop_token fires
};

// ---------------------------------------------------------------------------
// BindCallbackProgress — snapshot passed into the policy hook
// ---------------------------------------------------------------------------
struct BindCallbackProgress final {
    BindCallbackStatus  status{ BindCallbackStatus::BEGINNING_DOWNLOAD };
    std::uint64_t       bytesRead{};
    std::uint64_t       bytesTotal{};     ///< 0 if content-length unknown
    std::uint32_t       callCount{};      ///< How many OnProgress() calls so far
};

// ---------------------------------------------------------------------------
// BindCallbackPolicy — return value from CancelAwareBindCallback::ShouldCancel
// ---------------------------------------------------------------------------
enum class BindCallbackPolicy : std::uint8_t {
    CONTINUE = 0,   ///< Allow bind to proceed (return S_OK)
    CANCEL   = 1,   ///< Abort bind (return E_ABORT → HRESULT 0x80004004)
};

// ---------------------------------------------------------------------------
// CancelAwareBindCallback
// ---------------------------------------------------------------------------
// Header-only contract layer.  The concrete COM implementation
// (CancelAwareBindCallback.cpp) inherits this and implements IBindStatusCallback.
//
class CancelAwareBindCallback {
public:
    explicit CancelAwareBindCallback(DecodeTokenView tokenView) noexcept
        : m_tokenView{ tokenView }
        , m_callCount{ 0u }
    {}

    virtual ~CancelAwareBindCallback() = default;

    // Non-copyable.
    CancelAwareBindCallback(const CancelAwareBindCallback&)            = delete;
    CancelAwareBindCallback& operator=(const CancelAwareBindCallback&) = delete;

    // ── Core policy ──────────────────────────────────────────────────────────

    /// Check whether the decode token has been cancelled.
    /// Called from OnProgress() in the concrete COM class.
    [[nodiscard]] BindCallbackPolicy ShouldCancel(
        const BindCallbackProgress& progress) noexcept
    {
        m_callCount.fetch_add(1u, std::memory_order_relaxed);
        (void)progress;
        return m_tokenView.IsCancellationRequested()
            ? BindCallbackPolicy::CANCEL
            : BindCallbackPolicy::CONTINUE;
    }

    // ── Observers ────────────────────────────────────────────────────────────

    [[nodiscard]] bool IsCancelled() const noexcept
    {
        return m_tokenView.IsCancellationRequested();
    }

    [[nodiscard]] std::uint32_t CallCount() const noexcept
    {
        return m_callCount.load(std::memory_order_relaxed);
    }

    [[nodiscard]] const DecodeTokenView& TokenView() const noexcept
    {
        return m_tokenView;
    }

    // ── Constants ─────────────────────────────────────────────────────────────

    /// Maximum OnProgress() calls before we force a cancellation check
    /// regardless of token state (defensive against polling overflow).
    static constexpr std::uint32_t kMaxProgressCallsBeforeForceCheck = 1024u;

    /// HRESULT E_ABORT — what OnProgress() must return to cancel the bind.
    static constexpr std::int32_t kHResultAbort = static_cast<std::int32_t>(0x80004004);

protected:
    DecodeTokenView            m_tokenView;
    std::atomic<std::uint32_t> m_callCount;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_CANCEL_AWARE_BIND_CALLBACK_H
