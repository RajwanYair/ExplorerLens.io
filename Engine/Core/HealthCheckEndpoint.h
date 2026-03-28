#pragma once
// HealthCheckEndpoint.h — Health Check Endpoint
// HTTP-style health check API for monitoring tools — exposes liveness,
// readiness, and detailed component health via named pipe or local socket.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Health status
enum class EndpointHealthStatus : uint8_t {
 Healthy = 0, // All systems nominal
 Degraded, // Partial functionality
 Unhealthy, // Critical component failures
 Starting, // Initialization in progress
 ShuttingDown, // Graceful shutdown
 COUNT
};

/// Health check component
enum class HealthComponent : uint8_t {
 DecodePipeline = 0,
 GPURenderer,
 CacheSubsystem,
 PluginHost,
 MemoryManager,
 DiskIO,
 NetworkConnectivity,
 COMRegistration,
 COUNT
};

struct ComponentHealth {
 HealthComponent component = HealthComponent::DecodePipeline;
 EndpointHealthStatus status = EndpointHealthStatus::Healthy;
 const wchar_t *message = nullptr;
 double latencyMs = 0.0;
 uint64_t lastCheckTicks = 0;
};

struct OverallHealth {
 EndpointHealthStatus overallStatus = EndpointHealthStatus::Healthy;
 uint32_t healthyComponents = 0;
 uint32_t degradedComponents = 0;
 uint32_t unhealthyComponents = 0;
 uint64_t uptimeTicks = 0;
 uint32_t version = 15;
};

class HealthCheckEndpoint {
public:
 static constexpr size_t StatusCount() {
 return static_cast<size_t>(EndpointHealthStatus::COUNT);
 }
 static constexpr size_t ComponentCount() {
 return static_cast<size_t>(HealthComponent::COUNT);
 }

 static const wchar_t *StatusName(EndpointHealthStatus s) {
 switch (s) {
 case EndpointHealthStatus::Healthy:
 return L"Healthy";
 case EndpointHealthStatus::Degraded:
 return L"Degraded";
 case EndpointHealthStatus::Unhealthy:
 return L"Unhealthy";
 case EndpointHealthStatus::Starting:
 return L"Starting";
 case EndpointHealthStatus::ShuttingDown:
 return L"Shutting Down";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *ComponentName(HealthComponent c) {
 switch (c) {
 case HealthComponent::DecodePipeline:
 return L"Decode Pipeline";
 case HealthComponent::GPURenderer:
 return L"GPU Renderer";
 case HealthComponent::CacheSubsystem:
 return L"Cache Subsystem";
 case HealthComponent::PluginHost:
 return L"Plugin Host";
 case HealthComponent::MemoryManager:
 return L"Memory Manager";
 case HealthComponent::DiskIO:
 return L"Disk I/O";
 case HealthComponent::NetworkConnectivity:
 return L"Network";
 case HealthComponent::COMRegistration:
 return L"COM Registration";
 default:
 return L"Unknown";
 }
 }

 /// Aggregate component statuses into overall health
 static EndpointHealthStatus AggregateHealth(uint32_t healthy, uint32_t degraded,
 uint32_t unhealthy) {
 (void)healthy; // Used implicitly as total context
 if (unhealthy > 0)
 return EndpointHealthStatus::Unhealthy;
 if (degraded > 0)
 return EndpointHealthStatus::Degraded;
 return EndpointHealthStatus::Healthy;
 }
};

} // namespace Engine
} // namespace ExplorerLens
