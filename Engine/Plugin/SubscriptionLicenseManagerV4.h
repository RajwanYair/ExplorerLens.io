// SubscriptionLicenseManagerV4.h — Subscription & Seat-Based License Manager v4
// Copyright (c) 2026 ExplorerLens Project
//
// JWT-backed license validation with seat tracking for Individual/Team/Enterprise tiers.
//
#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens::Engine {

enum class SubscriptionLicenseTier : uint8_t {
    Free = 0,
    Individual = 1,
    Team = 2,
    Enterprise = 3,
    OEM = 4,
};

struct SeatAllocation
{
    uint32_t total = 0;
    uint32_t used = 0;
    uint32_t reserved = 0;
    uint32_t expired = 0;

    [[nodiscard]] uint32_t Available() const noexcept
    {
        return (total > used + reserved) ? (total - used - reserved) : 0u;
    }
    [[nodiscard]] bool HasCapacity() const noexcept
    {
        return Available() > 0;
    }
};

struct JWTEntitlement
{
    std::string subject;
    std::string issuer;
    std::chrono::system_clock::time_point expiry{};
    std::vector<std::string> features;
    std::string signature;  // Base64url-encoded HMAC-SHA256

    [[nodiscard]] bool IsExpired() const noexcept
    {
        return std::chrono::system_clock::now() > expiry;
    }
    [[nodiscard]] bool HasFeature(const std::string& feat) const noexcept;
};

struct SeatLease
{
    std::string leaseId;
    std::string userId;
    std::string pluginId;
    std::chrono::system_clock::time_point grantedAt{};
    std::chrono::system_clock::time_point expiresAt{};
};

class SubscriptionLicenseManagerV4
{
  public:
    explicit SubscriptionLicenseManagerV4(SubscriptionLicenseTier type = SubscriptionLicenseTier::Individual);
    ~SubscriptionLicenseManagerV4() noexcept;

    SubscriptionLicenseManagerV4(const SubscriptionLicenseManagerV4&) = delete;
    SubscriptionLicenseManagerV4& operator=(const SubscriptionLicenseManagerV4&) = delete;
    SubscriptionLicenseManagerV4(SubscriptionLicenseManagerV4&&) = default;
    SubscriptionLicenseManagerV4& operator=(SubscriptionLicenseManagerV4&&) = default;

    // Parse and cryptographically validate a JWT token string.
    [[nodiscard]] std::optional<JWTEntitlement> ValidateToken(const std::string& token) const;

    // Returns true if a seat is available for the given plugin / user combination.
    [[nodiscard]] bool CheckSeat(const std::string& pluginId, const std::string& userId) const;

    // Allocate a seat lease; returns nullopt if no capacity.
    [[nodiscard]] std::optional<SeatLease> AllocateSeat(const std::string& pluginId, const std::string& userId);

    // Release a seat by lease ID.
    bool ReleaseSeat(const std::string& leaseId) noexcept;

    // Obtain a refreshed JWT from the license server using the current token.
    [[nodiscard]] std::string RefreshToken(const std::string& currentToken) const;

    [[nodiscard]] SeatAllocation GetSeatAllocation(const std::string& pluginId) const;
    [[nodiscard]] SubscriptionLicenseTier GetLicenseType() const noexcept
    {
        return m_type;
    }
    [[nodiscard]] std::vector<SeatLease> GetActiveLeases() const;

    void SetLicenseEndpoint(const std::string& url);
    void SetHmacSecret(std::array<uint8_t, 32> secret) noexcept;

  private:
    SubscriptionLicenseTier m_type;
    std::string m_licenseEndpoint = "https://license.explorerlens.io/v4";
    std::array<uint8_t, 32> m_hmacSecret{};

    struct Impl
    {};
    std::unique_ptr<Impl> m_impl;

    [[nodiscard]] bool VerifySignature(const JWTEntitlement& ent, const std::string& rawToken) const noexcept;
    void ExpireLeases() noexcept;
};

}  // namespace ExplorerLens::Engine
