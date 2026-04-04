// EnterpriseConsoleV3.h — Console v3 REST + gRPC Admin API
// Copyright (c) 2026 ExplorerLens Project
//
// Provides EnterpriseConsoleV3 for remote administration via REST and gRPC,
// supporting multi-role access control and policy broadcast across managed fleets.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens::Engine {

enum class ConsoleProtocol : uint8_t {
    REST = 0,
    gRPC = 1,
    Hybrid = 2
};

enum class AdminRole : uint8_t {
    Viewer = 0,
    Operator = 1,
    Admin = 2,
    SuperAdmin = 3
};

struct ConsoleEndpoint
{
    std::string host;
    uint16_t port{443};
    ConsoleProtocol protocol{ConsoleProtocol::REST};
    std::string authToken;
    bool tlsEnabled{true};
};

struct EndpointStatus
{
    bool connected{false};
    uint32_t activeConnections{0};
    std::string serverVersion;
    std::chrono::system_clock::time_point lastPing;
    uint32_t uptimeSeconds{0};
};

struct RoleAssignment
{
    std::string userId;
    AdminRole role{AdminRole::Viewer};
    std::chrono::system_clock::time_point assignedAt;
};

class EnterpriseConsoleV3
{
  public:
    explicit EnterpriseConsoleV3(ConsoleEndpoint endpoint);
    ~EnterpriseConsoleV3();

    EnterpriseConsoleV3(const EnterpriseConsoleV3&) = delete;
    EnterpriseConsoleV3& operator=(const EnterpriseConsoleV3&) = delete;
    EnterpriseConsoleV3(EnterpriseConsoleV3&&) = default;
    EnterpriseConsoleV3& operator=(EnterpriseConsoleV3&&) = default;

    // Connection lifecycle
    bool Connect();
    void Disconnect();
    bool IsConnected() const noexcept;

    // Endpoint status
    EndpointStatus GetEndpointStatus() const;
    std::string GetServerVersion() const;

    // Role management
    bool SetRole(const std::string& userId, AdminRole role);
    std::optional<AdminRole> GetRole(const std::string& userId) const;
    std::vector<RoleAssignment> ListRoles() const;
    bool RevokeRole(const std::string& userId);

    // Policy distribution
    bool BroadcastPolicy(const std::string& policyJson);
    bool BroadcastPolicy(const std::string& policyJson, const std::vector<std::string>& targetEndpoints);

    // Asynchronous status events
    using StatusCallback = std::function<void(const EndpointStatus&)>;
    void SetStatusCallback(StatusCallback cb);

    // Diagnostics
    std::string GetDiagnosticsJson() const;
    bool PingServer() const;

  private:
    ConsoleEndpoint m_endpoint;
    bool m_connected{false};
    AdminRole m_ownRole{AdminRole::Viewer};
    StatusCallback m_statusCallback;

    bool AuthenticateToken() const;
    bool NegotiateProtocol();
    void ResetState();
    bool ValidateEndpoint() const;
};

}  // namespace ExplorerLens::Engine
