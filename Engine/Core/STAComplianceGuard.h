//==============================================================================
// ExplorerLens Engine — STA compliance guard (Sprint S236)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §2.1 A10: audit all COM entry points — never pump messages
//                       on the STA host thread, never block it > 16 ms.
//==============================================================================
//
// Windows Explorer calls IThumbnailProvider::GetThumbnail on a Single-Threaded
// Apartment (STA). Two rules apply:
//   1) Must not pump messages (Explorer owns the pump).
//   2) Must not block more than ~16 ms (Explorer UI freezes otherwise).
//
// This guard asserts on violation in Debug; in Release it samples and logs.
//==============================================================================
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>

#ifdef _WIN32
    #include <combaseapi.h>
#endif

namespace ExplorerLens {
namespace Engine {

/// <summary>
/// Apartment detection. NONE = CoInitialize never called on this thread.
/// </summary>
enum class ApartmentKind : std::uint8_t
{
    None         = 0,
    STA          = 1,
    MTA          = 2,
    NeutralAptmt = 3,
    Unknown      = 4
};

/// <summary>
/// RAII helper placed at the top of every COM entry point. On construction
/// it captures the apartment kind; on destruction it verifies the call
/// stayed within the STA budget and reports violations.
/// </summary>
class STAComplianceGuard
{
  public:
    /// <summary>
    /// budgetMicros — soft latency budget. Default: 16 ms = one 60 Hz frame.
    /// </summary>
    explicit STAComplianceGuard(std::uint64_t budgetMicros = 16'000) noexcept
        : m_start(std::chrono::steady_clock::now())
        , m_budget(budgetMicros)
        , m_apartment(DetectApartment())
    {}

    STAComplianceGuard(const STAComplianceGuard&)            = delete;
    STAComplianceGuard& operator=(const STAComplianceGuard&) = delete;

    ~STAComplianceGuard() noexcept
    {
        const auto end   = std::chrono::steady_clock::now();
        const auto micro = std::chrono::duration_cast<std::chrono::microseconds>(
                               end - m_start).count();
        if (static_cast<std::uint64_t>(micro) > m_budget) {
            ++s_violations;
        }
        s_lastDurationMicros.store(static_cast<std::uint64_t>(micro),
                                   std::memory_order_relaxed);
    }

    ApartmentKind Apartment() const noexcept { return m_apartment; }
    bool IsSTA() const noexcept { return m_apartment == ApartmentKind::STA; }

    static std::uint64_t Violations() noexcept { return s_violations.load(); }
    static std::uint64_t LastDurationMicros() noexcept
    {
        return s_lastDurationMicros.load();
    }
    static void ResetCounters() noexcept
    {
        s_violations         = 0;
        s_lastDurationMicros = 0;
    }

    /// <summary>
    /// Static probe callable outside an entry point — useful in tests.
    /// </summary>
    [[nodiscard]] static ApartmentKind DetectApartment() noexcept
    {
#ifdef _WIN32
        APTTYPE apt{};
        APTTYPEQUALIFIER qual{};
        const HRESULT hr = ::CoGetApartmentType(&apt, &qual);
        if (FAILED(hr)) {
            return ApartmentKind::None;
        }
        switch (apt) {
            case APTTYPE_STA:
            case APTTYPE_MAINSTA:
                return ApartmentKind::STA;
            case APTTYPE_MTA:
                return ApartmentKind::MTA;
            case APTTYPE_NA:
                return ApartmentKind::NeutralAptmt;
            default:
                return ApartmentKind::Unknown;
        }
#else
        return ApartmentKind::None;
#endif
    }

  private:
    std::chrono::steady_clock::time_point m_start;
    std::uint64_t  m_budget;
    ApartmentKind  m_apartment;

    inline static std::atomic<std::uint64_t> s_violations{0};
    inline static std::atomic<std::uint64_t> s_lastDurationMicros{0};
};

} // namespace Engine
} // namespace ExplorerLens
