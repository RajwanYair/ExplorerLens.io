#include "shellhost_client.h"

#include <algorithm>
#include <cstring>
#include <cmath>
#include <numeric>
#include <sstream>
#include <thread>

namespace ExplorerLens {
namespace ShellHost {

ShellHostClient::ShellHostClient(const Worker::WorkerConfig& workerConfig)
    : m_workerConfig(workerConfig) {
}

ShellHostClient::~ShellHostClient() {
    Shutdown();
}

bool ShellHostClient::Initialize() {
    if (!m_worker) {
        m_worker = std::make_unique<Worker::WorkerProcess>(m_workerConfig);
    }

    if (!m_ipcClient) {
        m_ipcClient = std::make_unique<IPC::IPCClient>(m_workerConfig.ipcConfig);
        m_ipcClient->SetAutoReconnect(true);
    }

    return m_worker->Start();
}

void ShellHostClient::Shutdown() {
    if (m_ipcClient) {
        m_ipcClient->Disconnect();
        m_ipcClient.reset();
    }

    if (m_worker) {
        m_worker->Stop();
        m_worker.reset();
    }
}

HRESULT ShellHostClient::GetThumbnail(const wchar_t* filePath,
                                      uint32_t sizePx,
                                      HBITMAP* phBitmap,
                                      uint32_t timeoutMs) {
    if (!filePath || !phBitmap) {
        return E_INVALIDARG;
    }

    ++m_totalRequests;

    if (!ShouldAllowRequest()) {
        ++m_failedRequests;
        if (m_fallbackEnabled) {
            ++m_fallbackRequests;
            return GetThumbnailLegacy(filePath, sizePx, phBitmap);
        }
        return HRESULT_FROM_WIN32(ERROR_RETRY);
    }

    if (!EnsureWorkerRunning()) {
        RecordFailure();
        ++m_failedRequests;
        if (m_fallbackEnabled) {
            ++m_fallbackRequests;
            return GetThumbnailLegacy(filePath, sizePx, phBitmap);
        }
        return HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE);
    }

    IPC::ThumbnailRequest request{};
    request.header = IPC::CreateHeader(IPC::MessageType::REQUEST_THUMBNAIL,
                                       IPC::GenerateMessageId(),
                                       static_cast<uint32_t>(sizeof(IPC::ThumbnailRequest) - sizeof(IPC::MessageHeader)) +
                                       static_cast<uint32_t>((wcslen(filePath) + 1) * sizeof(wchar_t)));
    request.sizePx = sizePx;
    request.flags = IPC::FLAG_ALLOW_GPU | IPC::FLAG_USE_CACHE;
    request.timeoutMs = timeoutMs;
    request.pathLength = static_cast<uint32_t>((wcslen(filePath) + 1) * sizeof(wchar_t));

    IPC::ThumbnailResponse response{};
    bool ok = m_worker->SubmitRequest(request, response, timeoutMs);
    if (!ok) {
        RecordFailure();
        ++m_failedRequests;
        if (m_fallbackEnabled) {
            ++m_fallbackRequests;
            return GetThumbnailLegacy(filePath, sizePx, phBitmap);
        }
        return E_FAIL;
    }

    std::vector<uint8_t> pixels;
    HBITMAP bmp = ConvertResponseToBitmap(response, pixels);
    if (!bmp) {
        RecordFailure();
        ++m_failedRequests;
        if (m_fallbackEnabled) {
            ++m_fallbackRequests;
            return GetThumbnailLegacy(filePath, sizePx, phBitmap);
        }
        return E_FAIL;
    }

    *phBitmap = bmp;
    RecordSuccess();
    ++m_successfulRequests;
    return S_OK;
}

HRESULT ShellHostClient::GetThumbnailLegacy(const wchar_t* filePath,
                                            uint32_t sizePx,
                                            HBITMAP* phBitmap) {
    return Legacy::GenerateThumbnailInProcess(filePath, sizePx, phBitmap);
}

bool ShellHostClient::IsWorkerHealthy() const {
    return m_worker && m_worker->IsHealthy();
}

void ShellHostClient::SetCircuitBreakerConfig(const CircuitBreakerConfig& config) {
    std::lock_guard<std::mutex> lock(m_circuitMutex);
    m_circuitConfig = config;
}

bool ShellHostClient::ShouldAllowRequest() {
    const CircuitState current = m_circuitState.load();
    if (current == CircuitState::CLOSED) {
        return true;
    }

    if (current == CircuitState::OPEN) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - m_circuitOpenTime).count();
        if (elapsed >= m_circuitConfig.openDurationMs) {
            TransitionToHalfOpen();
            return true;
        }
        return false;
    }

    return m_consecutiveSuccesses.load() < m_circuitConfig.halfOpenMaxAttempts;
}

void ShellHostClient::RecordSuccess() {
    m_consecutiveFailures = 0;

    if (m_circuitState == CircuitState::HALF_OPEN) {
        const uint32_t successes = ++m_consecutiveSuccesses;
        if (successes >= m_circuitConfig.successThreshold) {
            CloseCircuit();
        }
    }
}

void ShellHostClient::RecordFailure() {
    m_consecutiveSuccesses = 0;
    const uint32_t failures = ++m_consecutiveFailures;

    if (m_circuitState == CircuitState::HALF_OPEN || failures >= m_circuitConfig.failureThreshold) {
        OpenCircuit();
    }
}

void ShellHostClient::OpenCircuit() {
    std::lock_guard<std::mutex> lock(m_circuitMutex);
    m_circuitState = CircuitState::OPEN;
    m_circuitOpenTime = std::chrono::steady_clock::now();
    ++m_circuitOpenCount;
}

void ShellHostClient::CloseCircuit() {
    std::lock_guard<std::mutex> lock(m_circuitMutex);
    m_circuitState = CircuitState::CLOSED;
    m_consecutiveFailures = 0;
    m_consecutiveSuccesses = 0;
}

void ShellHostClient::TransitionToHalfOpen() {
    std::lock_guard<std::mutex> lock(m_circuitMutex);
    m_circuitState = CircuitState::HALF_OPEN;
    m_consecutiveSuccesses = 0;
}

bool ShellHostClient::EnsureWorkerRunning() {
    if (!m_worker) {
        m_worker = std::make_unique<Worker::WorkerProcess>(m_workerConfig);
    }

    if (m_worker->IsRunning()) {
        return true;
    }

    return m_worker->Start();
}

bool ShellHostClient::RestartWorker() {
    return m_worker && m_worker->Restart();
}

HRESULT ShellHostClient::SendThumbnailRequest(const wchar_t* filePath,
                                              uint32_t sizePx,
                                              IPC::ThumbnailResponse& response,
                                              uint32_t timeoutMs) {
    if (!m_worker) {
        return E_FAIL;
    }

    IPC::ThumbnailRequest request{};
    request.header = IPC::CreateHeader(IPC::MessageType::REQUEST_THUMBNAIL,
                                       IPC::GenerateMessageId(),
                                       static_cast<uint32_t>(sizeof(IPC::ThumbnailRequest) - sizeof(IPC::MessageHeader)));
    request.sizePx = sizePx;
    request.flags = IPC::FLAG_ALLOW_GPU | IPC::FLAG_USE_CACHE;
    request.timeoutMs = timeoutMs;
    request.pathLength = static_cast<uint32_t>((wcslen(filePath) + 1) * sizeof(wchar_t));

    return m_worker->SubmitRequest(request, response, timeoutMs) ? S_OK : E_FAIL;
}

HBITMAP ShellHostClient::ConvertResponseToBitmap(const IPC::ThumbnailResponse& response,
                                                 const std::vector<uint8_t>& pixelData) {
    if (response.width == 0 || response.height == 0) {
        return nullptr;
    }

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(response.width);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(response.height);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP bmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (!bmp || !bits) {
        return nullptr;
    }

    const size_t requiredBytes = static_cast<size_t>(response.width) * response.height * 4;
    if (pixelData.size() >= requiredBytes) {
        memcpy(bits, pixelData.data(), requiredBytes);
    } else {
        memset(bits, 0xCD, requiredBytes);
    }

    return bmp;
}

TimeoutGuard::TimeoutGuard(uint32_t timeoutMs)
    : m_startTime(std::chrono::steady_clock::now()), m_timeout(timeoutMs) {
}

TimeoutGuard::~TimeoutGuard() = default;

bool TimeoutGuard::IsExpired() const {
    return std::chrono::steady_clock::now() - m_startTime >= m_timeout;
}

uint32_t TimeoutGuard::RemainingMs() const {
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - m_startTime);
    if (elapsed >= m_timeout) {
        return 0;
    }
    return static_cast<uint32_t>((m_timeout - elapsed).count());
}

RequestQueueManager::RequestQueueManager(uint32_t maxQueueSize)
    : m_maxQueueSize(maxQueueSize) {
}

bool RequestQueueManager::Enqueue(const RequestContext& context) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queueDepth >= m_maxQueueSize) {
        return false;
    }
    m_queue.push(context);
    ++m_queueDepth;
    m_cv.notify_one();
    return true;
}

bool RequestQueueManager::Dequeue(RequestContext& context, uint32_t timeoutMs) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (timeoutMs == 0) {
        m_cv.wait(lock, [this]() { return !m_queue.empty(); });
    } else {
        if (!m_cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return !m_queue.empty(); })) {
            return false;
        }
    }

    context = m_queue.front();
    m_queue.pop();
    if (m_queueDepth > 0) {
        --m_queueDepth;
    }
    return true;
}

bool RequestQueueManager::Cancel(uint64_t correlationId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::queue<RequestContext> filtered;
    bool found = false;

    while (!m_queue.empty()) {
        auto item = m_queue.front();
        m_queue.pop();
        if (item.correlationId == correlationId) {
            found = true;
            if (m_queueDepth > 0) {
                --m_queueDepth;
            }
            continue;
        }
        filtered.push(item);
    }

    m_queue = std::move(filtered);
    return found;
}

void RequestQueueManager::RemoveExpiredRequests() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::queue<RequestContext> filtered;

    while (!m_queue.empty()) {
        auto item = m_queue.front();
        m_queue.pop();
        if (!item.IsTimedOut()) {
            filtered.push(item);
        } else if (m_queueDepth > 0) {
            --m_queueDepth;
        }
    }

    m_queue = std::move(filtered);
}

void RequestQueueManager::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_queue.empty()) {
        m_queue.pop();
    }
    m_queueDepth = 0;
}

RetryPolicy::RetryPolicy(const RetryConfig& config)
    : m_config(config) {
}

bool RetryPolicy::ShouldRetry(IPC::ErrorCode errorCode, uint32_t attemptCount) const {
    if (attemptCount >= m_config.maxAttempts) {
        return false;
    }

    if (errorCode == IPC::ErrorCode::IPC_TIMEOUT) {
        return m_config.retryOnTimeout;
    }

    if (errorCode == IPC::ErrorCode::IPC_CONNECTION_FAILED || errorCode == IPC::ErrorCode::IPC_PIPE_BROKEN) {
        return m_config.retryOnConnectionError;
    }

    return false;
}

uint32_t RetryPolicy::GetBackoffMs(uint32_t attemptCount) const {
    const double backoff = static_cast<double>(m_config.initialBackoffMs) * std::pow(m_config.backoffMultiplier, attemptCount);
    return static_cast<uint32_t>(std::min(backoff, static_cast<double>(m_config.maxBackoffMs)));
}

void MetricsCollector::RecordRequest(const PerformanceMetrics& metrics) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_metrics.push_back(metrics);
    if (m_metrics.size() > MAX_METRICS) {
        m_metrics.erase(m_metrics.begin(), m_metrics.begin() + (m_metrics.size() - MAX_METRICS));
    }
}

double MetricsCollector::GetAverageRequestTimeMs() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_metrics.empty()) return 0.0;
    const uint64_t sum = std::accumulate(m_metrics.begin(), m_metrics.end(), uint64_t{0},
        [](uint64_t acc, const PerformanceMetrics& m) { return acc + m.totalRequestTimeUs; });
    return static_cast<double>(sum) / (m_metrics.size() * 1000.0);
}

double MetricsCollector::GetAverageIPCOverheadMs() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_metrics.empty()) return 0.0;
    const uint64_t sum = std::accumulate(m_metrics.begin(), m_metrics.end(), uint64_t{0},
        [](uint64_t acc, const PerformanceMetrics& m) { return acc + m.ipcOverheadUs; });
    return static_cast<double>(sum) / (m_metrics.size() * 1000.0);
}

double MetricsCollector::GetSuccessRate() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_metrics.empty()) return 0.0;
    const size_t successCount = std::count_if(m_metrics.begin(), m_metrics.end(),
        [](const PerformanceMetrics& m) { return m.success; });
    return static_cast<double>(successCount) / static_cast<double>(m_metrics.size());
}

double MetricsCollector::GetFallbackRate() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_metrics.empty()) return 0.0;
    const size_t fallbackCount = std::count_if(m_metrics.begin(), m_metrics.end(),
        [](const PerformanceMetrics& m) { return m.fallbackUsed; });
    return static_cast<double>(fallbackCount) / static_cast<double>(m_metrics.size());
}

uint32_t MetricsCollector::GetP50LatencyMs() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_metrics.empty()) return 0;
    std::vector<uint64_t> vals;
    vals.reserve(m_metrics.size());
    for (const auto& m : m_metrics) vals.push_back(m.totalRequestTimeUs / 1000ULL);
    std::sort(vals.begin(), vals.end());
    return static_cast<uint32_t>(vals[vals.size() / 2]);
}

uint32_t MetricsCollector::GetP95LatencyMs() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_metrics.empty()) return 0;
    std::vector<uint64_t> vals;
    vals.reserve(m_metrics.size());
    for (const auto& m : m_metrics) vals.push_back(m.totalRequestTimeUs / 1000ULL);
    std::sort(vals.begin(), vals.end());
    const size_t idx = static_cast<size_t>(std::floor((vals.size() - 1) * 0.95));
    return static_cast<uint32_t>(vals[idx]);
}

uint32_t MetricsCollector::GetP99LatencyMs() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_metrics.empty()) return 0;
    std::vector<uint64_t> vals;
    vals.reserve(m_metrics.size());
    for (const auto& m : m_metrics) vals.push_back(m.totalRequestTimeUs / 1000ULL);
    std::sort(vals.begin(), vals.end());
    const size_t idx = static_cast<size_t>(std::floor((vals.size() - 1) * 0.99));
    return static_cast<uint32_t>(vals[idx]);
}

std::string MetricsCollector::ExportJSON() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    double avgRequestMs = 0.0;
    double avgIpcMs = 0.0;
    double successRate = 0.0;

    if (!m_metrics.empty()) {
        const uint64_t totalRequestUs = std::accumulate(m_metrics.begin(), m_metrics.end(), uint64_t{0},
            [](uint64_t acc, const PerformanceMetrics& m) { return acc + m.totalRequestTimeUs; });
        const uint64_t totalIpcUs = std::accumulate(m_metrics.begin(), m_metrics.end(), uint64_t{0},
            [](uint64_t acc, const PerformanceMetrics& m) { return acc + m.ipcOverheadUs; });
        const size_t successCount = std::count_if(m_metrics.begin(), m_metrics.end(),
            [](const PerformanceMetrics& m) { return m.success; });

        avgRequestMs = static_cast<double>(totalRequestUs) / (m_metrics.size() * 1000.0);
        avgIpcMs = static_cast<double>(totalIpcUs) / (m_metrics.size() * 1000.0);
        successRate = static_cast<double>(successCount) / static_cast<double>(m_metrics.size());
    }

    std::ostringstream os;
    os << "{";
    os << "\"count\":" << m_metrics.size() << ",";
    os << "\"avgRequestMs\":" << avgRequestMs << ",";
    os << "\"avgIpcMs\":" << avgIpcMs << ",";
    os << "\"successRate\":" << successRate;
    os << "}";
    return os.str();
}

void MetricsCollector::Reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_metrics.clear();
}

std::unique_ptr<ShellHostClient> ShellHostFactory::s_instance;
std::once_flag ShellHostFactory::s_initFlag;
Worker::WorkerConfig ShellHostFactory::s_workerConfig;
CircuitBreakerConfig ShellHostFactory::s_circuitConfig;

ShellHostClient& ShellHostFactory::GetInstance() {
    std::call_once(s_initFlag, []() {
        s_instance = std::make_unique<ShellHostClient>(s_workerConfig);
        s_instance->SetCircuitBreakerConfig(s_circuitConfig);
        s_instance->Initialize();
    });
    return *s_instance;
}

void ShellHostFactory::Shutdown() {
    if (s_instance) {
        s_instance->Shutdown();
        s_instance.reset();
    }
}

void ShellHostFactory::Configure(const Worker::WorkerConfig& workerConfig,
                                 const CircuitBreakerConfig& circuitConfig) {
    s_workerConfig = workerConfig;
    s_circuitConfig = circuitConfig;
}

namespace Legacy {

HRESULT GenerateThumbnailInProcess(const wchar_t* filePath,
                                   uint32_t sizePx,
                                   HBITMAP* phBitmap) {
    if (!filePath || !phBitmap || sizePx == 0) {
        return E_INVALIDARG;
    }

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(sizePx);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(sizePx);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP bmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (!bmp || !bits) {
        return E_OUTOFMEMORY;
    }

    memset(bits, 0xAA, static_cast<size_t>(sizePx) * sizePx * 4);
    *phBitmap = bmp;
    return S_OK;
}

bool IsLegacyPathAvailable() {
    return true;
}

} // namespace Legacy

} // namespace ShellHost
} // namespace ExplorerLens

