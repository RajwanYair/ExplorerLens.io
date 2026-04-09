// IdleTimePreGenerator.cpp — CPU/GPU Idle-Time Thumbnail Pre-Generator
// Copyright (c) 2026 ExplorerLens Project
//
#include "Pipeline/IdleTimePreGenerator.h"
#include <atomic>
#include <thread>
#include <chrono>

#if defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

namespace ExplorerLens { namespace Engine {

struct IdleTimePreGenerator::Impl {
    std::thread          thread;
    std::atomic<bool>    stop      { false };
    std::atomic<uint64_t> generated { 0 };
    std::atomic<uint64_t> windows   { 0 };
    float lastCpu     = 0.0f;
    float lastGpu     = 0.0f;
    float lastThermal = 0.0f;
};

float IdleTimePreGenerator::SampleCpuPercent() noexcept
{
#if defined(_WIN32)
    static ULARGE_INTEGER prevIdle{}, prevKernel{}, prevUser{};
    FILETIME idleTime{}, kernelTime{}, userTime{};
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) return 0.0f;

    ULARGE_INTEGER idle{}, kernel{}, user{};
    idle.LowPart    = idleTime.dwLowDateTime;
    idle.HighPart   = idleTime.dwHighDateTime;
    kernel.LowPart  = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart    = userTime.dwLowDateTime;
    user.HighPart   = userTime.dwHighDateTime;

    const ULONGLONG sysTotal = (kernel.QuadPart - prevKernel.QuadPart)
                             + (user.QuadPart   - prevUser.QuadPart);
    const ULONGLONG sysIdle  =  idle.QuadPart   - prevIdle.QuadPart;

    prevIdle   = idle;
    prevKernel = kernel;
    prevUser   = user;

    if (sysTotal == 0) return 0.0f;
    const float cpuUsed = 100.0f * (1.0f - static_cast<float>(sysIdle) /
                                           static_cast<float>(sysTotal));
    return cpuUsed > 0.0f ? cpuUsed : 0.0f;
#else
    return 0.0f;
#endif
}

float IdleTimePreGenerator::SampleGpuPercent() noexcept
{
    // Placeholder — a full GPU utilisation sampler via DXGI/PDH would go here.
    return 0.0f;
}

float IdleTimePreGenerator::SampleThermalCelsius() noexcept
{
    return 0.0f;
}

bool IdleTimePreGenerator::IsOnBattery() noexcept
{
#if defined(_WIN32)
    SYSTEM_POWER_STATUS sps{};
    if (!GetSystemPowerStatus(&sps)) return false;
    return sps.ACLineStatus == 0;
#else
    return false;
#endif
}

IdleTimePreGenerator::IdleTimePreGenerator(
    const IdlePreGenConfig& config) noexcept
    : m_config(config)
{
    m_impl = new Impl{};
}

IdleTimePreGenerator::~IdleTimePreGenerator()
{
    Stop();
    delete m_impl;
}

void IdleTimePreGenerator::SetWorkCallback(WorkCallback cb) noexcept
{
    m_workCb = std::move(cb);
}

void IdleTimePreGenerator::Start() noexcept
{
    if (m_running) return;
    m_running = true;
    m_impl->stop.store(false);

    m_impl->thread = std::thread([this]() {
#if defined(_WIN32)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
#endif
        while (!m_impl->stop.load()) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(m_config.pollIntervalMs));

            if (m_config.disableOnBattery && IsOnBattery()) continue;

            const float cpu     = SampleCpuPercent();
            const float gpu     = SampleGpuPercent();
            const float thermal = SampleThermalCelsius();

            m_impl->lastCpu     = cpu;
            m_impl->lastGpu     = gpu;
            m_impl->lastThermal = thermal;

            if (cpu     > m_config.cpuIdleThreshold)   continue;
            if (gpu     > m_config.gpuIdleThreshold)   continue;
            if (thermal > m_config.thermalThreshold &&
                          m_config.thermalThreshold > 0) continue;

            if (!m_workCb) continue;

            m_impl->windows.fetch_add(1);
            for (uint32_t i = 0; i < m_config.batchSize; ++i) {
                if (m_impl->stop.load()) break;
                if (!m_workCb()) break;
                m_impl->generated.fetch_add(1);
            }
        }
    });
}

void IdleTimePreGenerator::Stop() noexcept
{
    if (!m_running) return;
    m_impl->stop.store(true);
    if (m_impl->thread.joinable()) m_impl->thread.join();
    m_running = false;
}

IdlePreGenStats IdleTimePreGenerator::GetStats() const noexcept
{
    IdlePreGenStats s{};
    s.totalPreGenerated  = m_impl->generated.load();
    s.idleWindowsUsed    = m_impl->windows.load();
    s.lastCpuPercent     = m_impl->lastCpu;
    s.lastGpuPercent     = m_impl->lastGpu;
    s.lastThermalCelsius = m_impl->lastThermal;
    s.isActive           = m_running;
    return s;
}

bool IdleTimePreGenerator::IsRunning() const noexcept { return m_running; }

}} // namespace ExplorerLens::Engine
