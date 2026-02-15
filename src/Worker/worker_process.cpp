#include "worker_process.h"

#include <algorithm>
#include <cstring>
#include <sstream>

namespace DarkThumbs {
namespace Worker {

WorkerProcess::WorkerProcess(const WorkerConfig& config)
    : m_config(config) {
    m_stats.startTimeUs = IPC::GetTimestampUs();
    m_stats.currentState = WorkerState::STOPPED;
}

WorkerProcess::~WorkerProcess() {
    Stop();
}

bool WorkerProcess::Start() {
    if (m_running) {
        return true;
    }

    m_state = WorkerState::STARTING;
    m_stats.currentState = WorkerState::STARTING;

    if (!LaunchWorkerProcess()) {
        m_state = WorkerState::CRASHED;
        m_stats.currentState = WorkerState::CRASHED;
        return false;
    }

    m_ipcClient = std::make_unique<IPC::IPCClient>(m_config.ipcConfig);
    m_ipcClient->SetAutoReconnect(true);

    m_running = true;
    m_state = WorkerState::READY;
    m_stats.currentState = WorkerState::READY;
    m_lastHeartbeat = std::chrono::steady_clock::now();

    m_watchdogThread = std::make_unique<std::thread>(&WorkerProcess::WatchdogLoop, this);
    return true;
}

void WorkerProcess::Stop() {
    m_running = false;
    m_state = WorkerState::STOPPING;
    m_stats.currentState = WorkerState::STOPPING;

    if (m_watchdogThread && m_watchdogThread->joinable()) {
        m_watchdogThread->join();
    }
    m_watchdogThread.reset();

    if (m_ipcClient) {
        m_ipcClient->Disconnect();
        m_ipcClient.reset();
    }

    TerminateWorkerProcess();

    m_state = WorkerState::STOPPED;
    m_stats.currentState = WorkerState::STOPPED;
}

bool WorkerProcess::Restart() {
    Stop();
    ++m_stats.restartCount;
    return Start();
}

bool WorkerProcess::IsRunning() const {
    return m_running && m_state != WorkerState::STOPPED && m_state != WorkerState::CRASHED;
}

bool WorkerProcess::SubmitRequest(const IPC::ThumbnailRequest& request,
                                  IPC::ThumbnailResponse& response,
                                  uint32_t timeoutMs) {
    if (!IsRunning()) {
        ++m_stats.requestsFailed;
        return false;
    }

    const auto start = std::chrono::steady_clock::now();

    if (m_ipcClient && m_ipcClient->Connect(timeoutMs == 0 ? m_config.ipcConfig.connectTimeoutMs : timeoutMs)) {
        IPC::MessageHeader respHeader{};
        std::vector<uint8_t> payload;

        IPC::IPCStatus status = m_ipcClient->SendRequest(
            request.header,
            reinterpret_cast<const uint8_t*>(&request) + sizeof(IPC::MessageHeader),
            respHeader,
            payload,
            timeoutMs == 0 ? request.timeoutMs : timeoutMs
        );

        if (status == IPC::IPCStatus::OK && respHeader.payloadSize >= sizeof(IPC::ThumbnailResponse) - sizeof(IPC::MessageHeader)) {
            memcpy(&response, payload.data(), std::min(payload.size(), sizeof(IPC::ThumbnailResponse) - sizeof(IPC::MessageHeader)));
            ++m_stats.requestsProcessed;
            m_state = WorkerState::BUSY;
            m_stats.currentState = WorkerState::BUSY;
            UpdateHeartbeat();
        } else {
            ++m_stats.requestsFailed;
            return false;
        }
    } else {
        ++m_stats.requestsFailed;
        return false;
    }

    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - start).count();
    m_stats.totalProcessingTimeUs += static_cast<uint64_t>(elapsed);
    if (m_stats.requestsProcessed > 0) {
        m_stats.averageProcessingTimeMs = static_cast<uint32_t>((m_stats.totalProcessingTimeUs / m_stats.requestsProcessed) / 1000ULL);
    }

    m_state = WorkerState::READY;
    m_stats.currentState = WorkerState::READY;
    return true;
}

bool WorkerProcess::CancelRequest(uint32_t requestId) {
    if (!m_ipcClient || !m_ipcClient->IsConnected()) {
        return false;
    }

    IPC::CancelRequest cancel{};
    cancel.header = IPC::CreateHeader(IPC::MessageType::REQUEST_CANCEL, requestId, sizeof(IPC::CancelRequest) - sizeof(IPC::MessageHeader));
    cancel.requestIdToCancel = requestId;

    return m_ipcClient->Send(cancel.header, &cancel.requestIdToCancel) == IPC::IPCStatus::OK;
}

bool WorkerProcess::IsHealthy() const {
    if (!IsRunning()) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    const auto sinceHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastHeartbeat).count();
    return sinceHeartbeat < static_cast<long long>(m_config.idleTimeoutMs);
}

bool WorkerProcess::Ping(uint32_t timeoutMs) {
    if (!m_ipcClient) {
        return false;
    }

    if (!m_ipcClient->Connect(timeoutMs)) {
        return false;
    }

    IPC::PingRequest ping{};
    ping.header = IPC::CreateHeader(IPC::MessageType::REQUEST_PING, IPC::GenerateMessageId(), sizeof(IPC::PingRequest) - sizeof(IPC::MessageHeader));
    ping.sendTime = IPC::GetTimestampUs();

    IPC::MessageHeader responseHeader{};
    std::vector<uint8_t> payload;
    auto status = m_ipcClient->SendRequest(ping.header, &ping.sendTime, responseHeader, payload, timeoutMs);
    if (status != IPC::IPCStatus::OK) {
        return false;
    }

    UpdateHeartbeat();
    return true;
}

void WorkerProcess::UpdateConfig(const WorkerConfig& config) {
    m_config = config;
}

bool WorkerProcess::LaunchWorkerProcess() {
    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo{};
    std::wstring cmd = L"\"" + m_config.workerExePath + L"\"";

    BOOL ok = CreateProcessW(
        nullptr,
        cmd.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        m_config.workingDirectory.empty() ? nullptr : m_config.workingDirectory.c_str(),
        &startupInfo,
        &processInfo);

    if (!ok) {
        return false;
    }

    m_processHandle = processInfo.hProcess;
    m_processId = processInfo.dwProcessId;
    CloseHandle(processInfo.hThread);

    if (!SetupJobObject()) {
        return false;
    }

    return true;
}

void WorkerProcess::TerminateWorkerProcess() {
    if (m_processHandle) {
        TerminateProcess(m_processHandle, 0);
        CloseHandle(m_processHandle);
        m_processHandle = nullptr;
    }

    if (m_jobObject) {
        CloseHandle(m_jobObject);
        m_jobObject = nullptr;
    }

    m_processId = 0;
}

bool WorkerProcess::SetupJobObject() {
    m_jobObject = CreateJobObjectW(nullptr, nullptr);
    if (!m_jobObject) {
        return false;
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    limits.ProcessMemoryLimit = static_cast<SIZE_T>(m_config.maxMemoryMB) * 1024ULL * 1024ULL;
    limits.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;

    if (!SetInformationJobObject(m_jobObject, JobObjectExtendedLimitInformation, &limits, sizeof(limits))) {
        return false;
    }

    return AssignProcessToJobObject(m_jobObject, m_processHandle) != FALSE;
}

void WorkerProcess::WatchdogLoop() {
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (!m_running) {
            break;
        }

        if (m_processHandle) {
            DWORD exitCode = STILL_ACTIVE;
            if (GetExitCodeProcess(m_processHandle, &exitCode) && exitCode != STILL_ACTIVE) {
                m_state = WorkerState::CRASHED;
                m_stats.currentState = WorkerState::CRASHED;
                ++m_stats.crashCount;

                if (!RecoverFromCrash()) {
                    m_running = false;
                    break;
                }
            }
        }

        const auto now = std::chrono::steady_clock::now();
        m_stats.uptimeSeconds = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(now - std::chrono::steady_clock::time_point{}).count());
    }
}

void WorkerProcess::UpdateHeartbeat() {
    m_lastHeartbeat = std::chrono::steady_clock::now();
    m_missedHeartbeats = 0;
}

bool WorkerProcess::RecoverFromCrash() {
    if (!m_config.restartOnCrash || m_currentRestartCount >= m_config.maxRestarts) {
        return false;
    }

    ++m_currentRestartCount;
    std::this_thread::sleep_for(std::chrono::milliseconds(m_config.restartBackoffMs * m_currentRestartCount));
    TerminateWorkerProcess();
    return LaunchWorkerProcess();
}

WorkerPool::WorkerPool(const WorkerConfig& config, uint32_t poolSize)
    : m_config(config) {
    m_workers.reserve(poolSize);
    for (uint32_t i = 0; i < poolSize; ++i) {
        m_workers.push_back(std::make_unique<WorkerProcess>(m_config));
    }
}

WorkerPool::~WorkerPool() {
    Stop();
}

bool WorkerPool::Start() {
    bool allStarted = true;
    for (auto& worker : m_workers) {
        allStarted &= worker->Start();
    }
    return allStarted;
}

void WorkerPool::Stop() {
    for (auto& worker : m_workers) {
        worker->Stop();
    }
}

bool WorkerPool::Prewarm() {
    return Start();
}

WorkerProcess* WorkerPool::SelectWorkerRoundRobin() {
    if (m_workers.empty()) {
        return nullptr;
    }

    const uint32_t index = m_nextWorkerIndex.fetch_add(1) % static_cast<uint32_t>(m_workers.size());
    return m_workers[index].get();
}

WorkerProcess* WorkerPool::SelectWorkerLeastLoaded() {
    WorkerProcess* best = nullptr;
    uint32_t bestQueue = UINT32_MAX;

    for (const auto& worker : m_workers) {
        const auto& stats = worker->GetStats();
        if (stats.currentQueueDepth < bestQueue && worker->IsHealthy()) {
            bestQueue = stats.currentQueueDepth;
            best = worker.get();
        }
    }

    return best ? best : SelectWorkerRoundRobin();
}

WorkerProcess* WorkerPool::GetAvailableWorker() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return SelectWorkerLeastLoaded();
}

bool WorkerPool::SubmitRequest(const IPC::ThumbnailRequest& request,
                               IPC::ThumbnailResponse& response,
                               uint32_t timeoutMs) {
    WorkerProcess* worker = GetAvailableWorker();
    if (!worker) {
        return false;
    }
    return worker->SubmitRequest(request, response, timeoutMs);
}

uint32_t WorkerPool::GetActiveWorkers() const {
    uint32_t active = 0;
    for (const auto& worker : m_workers) {
        if (worker->IsRunning()) {
            ++active;
        }
    }
    return active;
}

uint32_t WorkerPool::GetAggregateQueueDepth() const {
    uint32_t total = 0;
    for (const auto& worker : m_workers) {
        total += worker->GetStats().currentQueueDepth;
    }
    return total;
}

void WorkerPool::HealthCheck() {
    for (auto& worker : m_workers) {
        if (!worker->IsHealthy()) {
            worker->Restart();
        }
    }
}

void WorkerPool::RestartUnhealthyWorkers() {
    HealthCheck();
}

WorkerWatchdog::WorkerWatchdog(WorkerPool& pool)
    : m_pool(pool) {
}

WorkerWatchdog::~WorkerWatchdog() {
    Stop();
}

void WorkerWatchdog::Start() {
    if (m_running) {
        return;
    }

    m_running = true;
    m_thread = std::make_unique<std::thread>(&WorkerWatchdog::MonitorLoop, this);
}

void WorkerWatchdog::Stop() {
    m_running = false;
    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }
    m_thread.reset();
}

void WorkerWatchdog::MonitorLoop() {
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_heartbeatIntervalMs));
        if (!m_running) {
            break;
        }

        m_pool.HealthCheck();
    }
}

} // namespace Worker
} // namespace DarkThumbs
