#include "ipc_transport.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <sstream>
#include <thread>

namespace DarkThumbs {
namespace IPC {

namespace {
constexpr DWORD kPollIntervalMs = 10;
}

NamedPipeConnection::NamedPipeConnection(HANDLE pipe, const TransportConfig& config)
    : m_pipe(pipe), m_config(config) {
}

NamedPipeConnection::~NamedPipeConnection() {
    Close();
}

IPCStatus NamedPipeConnection::WriteData(const void* data, uint32_t size) {
    if (!data || size == 0 || m_pipe == INVALID_HANDLE_VALUE || m_pipe == nullptr) {
        return IPCStatus::INVALID_PARAMETER;
    }

    const auto* bytes = static_cast<const uint8_t*>(data);
    uint32_t totalWritten = 0;
    while (totalWritten < size) {
        DWORD written = 0;
        if (!WriteFile(m_pipe, bytes + totalWritten, size - totalWritten, &written, nullptr)) {
            const DWORD err = GetLastError();
            if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA) {
                return IPCStatus::CONNECTION_CLOSED;
            }
            return IPCStatus::PROTOCOL_ERROR;
        }
        totalWritten += written;
    }

    m_bytesSent += size;
    return IPCStatus::OK;
}

IPCStatus NamedPipeConnection::ReadData(void* data, uint32_t size, uint32_t timeoutMs) {
    if (!data || size == 0 || m_pipe == INVALID_HANDLE_VALUE || m_pipe == nullptr) {
        return IPCStatus::INVALID_PARAMETER;
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs == 0 ? m_config.readTimeoutMs : timeoutMs);
    uint8_t* out = static_cast<uint8_t*>(data);
    uint32_t totalRead = 0;

    while (totalRead < size) {
        if (timeoutMs != INFINITE) {
            DWORD available = 0;
            if (!PeekNamedPipe(m_pipe, nullptr, 0, nullptr, &available, nullptr)) {
                const DWORD err = GetLastError();
                if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA) {
                    return IPCStatus::CONNECTION_CLOSED;
                }
                return IPCStatus::PROTOCOL_ERROR;
            }

            if (available == 0) {
                if (std::chrono::steady_clock::now() >= deadline) {
                    return IPCStatus::TIMEOUT;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(kPollIntervalMs));
                continue;
            }
        }

        DWORD read = 0;
        if (!ReadFile(m_pipe, out + totalRead, size - totalRead, &read, nullptr)) {
            const DWORD err = GetLastError();
            if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA) {
                return IPCStatus::CONNECTION_CLOSED;
            }
            return IPCStatus::PROTOCOL_ERROR;
        }

        totalRead += read;
    }

    m_bytesReceived += size;
    return IPCStatus::OK;
}

IPCStatus NamedPipeConnection::Send(const MessageHeader& header, const void* payload) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!ValidateHeader(header)) {
        return IPCStatus::PROTOCOL_ERROR;
    }

    IPCStatus status = WriteData(&header, static_cast<uint32_t>(sizeof(MessageHeader)));
    if (status != IPCStatus::OK) {
        return status;
    }

    if (header.payloadSize > 0) {
        if (!payload) {
            return IPCStatus::INVALID_PARAMETER;
        }

        status = WriteData(payload, header.payloadSize);
        if (status != IPCStatus::OK) {
            return status;
        }
    }

    ++m_messagesSent;
    return IPCStatus::OK;
}

IPCStatus NamedPipeConnection::Receive(MessageHeader& header, std::vector<uint8_t>& payload, uint32_t timeoutMs) {
    std::lock_guard<std::mutex> lock(m_mutex);

    IPCStatus status = ReadData(&header, static_cast<uint32_t>(sizeof(MessageHeader)), timeoutMs);
    if (status != IPCStatus::OK) {
        return status;
    }

    if (!ValidateHeader(header)) {
        return IPCStatus::PROTOCOL_ERROR;
    }

    payload.clear();
    if (header.payloadSize > 0) {
        payload.resize(header.payloadSize);
        status = ReadData(payload.data(), header.payloadSize, timeoutMs);
        if (status != IPCStatus::OK) {
            payload.clear();
            return status;
        }
    }

    ++m_messagesReceived;
    return IPCStatus::OK;
}

bool NamedPipeConnection::IsConnected() const {
    return m_pipe != nullptr && m_pipe != INVALID_HANDLE_VALUE;
}

void NamedPipeConnection::Close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_pipe && m_pipe != INVALID_HANDLE_VALUE) {
        FlushFileBuffers(m_pipe);
        DisconnectNamedPipe(m_pipe);
        CloseHandle(m_pipe);
        m_pipe = nullptr;
    }
}

IPCServer::IPCServer(const TransportConfig& config)
    : m_config(config), m_pipeName(config.pipeName), m_pipeHandle(nullptr) {
}

IPCServer::~IPCServer() {
    Stop();
}

HANDLE IPCServer::CreatePipeInstance() {
    return CreateNamedPipeW(
        m_pipeName.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        m_config.maxInstances,
        m_config.pipeBufferSize,
        m_config.pipeBufferSize,
        0,
        nullptr
    );
}

bool IPCServer::Start() {
    if (m_running) {
        return true;
    }

    m_pipeHandle = CreatePipeInstance();
    if (m_pipeHandle == INVALID_HANDLE_VALUE) {
        m_pipeHandle = nullptr;
        return false;
    }

    m_running = true;
    return true;
}

void IPCServer::Stop() {
    m_running = false;
    if (m_pipeHandle && m_pipeHandle != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(m_pipeHandle);
        CloseHandle(m_pipeHandle);
        m_pipeHandle = nullptr;
    }
}

std::unique_ptr<IPCConnection> IPCServer::Accept(uint32_t timeoutMs) {
    if (!m_running || !m_pipeHandle || m_pipeHandle == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    if (timeoutMs != INFINITE && timeoutMs > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    BOOL connected = ConnectNamedPipe(m_pipeHandle, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    if (!connected) {
        return nullptr;
    }

    auto conn = std::make_unique<NamedPipeConnection>(m_pipeHandle, m_config);
    m_pipeHandle = CreatePipeInstance();
    return conn;
}

void IPCServer::SetConnectionCallback(ConnectionCallback callback) {
    m_connectionCallback = std::move(callback);
}

bool IPCServer::StartAsync() {
    if (!Start()) {
        return false;
    }

    if (!m_connectionCallback) {
        return true;
    }

    std::thread([this]() {
        while (m_running) {
            auto connection = Accept(1000);
            if (connection && m_connectionCallback) {
                m_connectionCallback(std::move(connection));
            }
        }
    }).detach();

    return true;
}

IPCClient::IPCClient(const TransportConfig& config)
    : m_config(config), m_pipeName(config.pipeName) {
}

IPCClient::~IPCClient() {
    Disconnect();
}

HANDLE IPCClient::ConnectToPipe(uint32_t timeoutMs) {
    const uint32_t effectiveTimeout = timeoutMs == 0 ? m_config.connectTimeoutMs : timeoutMs;
    if (!WaitForPipe(m_pipeName, effectiveTimeout)) {
        return nullptr;
    }

    return CreateFileW(
        m_pipeName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
}

bool IPCClient::Connect(uint32_t timeoutMs) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_connection && m_connection->IsConnected()) {
        return true;
    }

    HANDLE pipe = ConnectToPipe(timeoutMs);
    if (!pipe || pipe == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD mode = PIPE_READMODE_MESSAGE;
    SetNamedPipeHandleState(pipe, &mode, nullptr, nullptr);

    m_connection = std::make_unique<NamedPipeConnection>(pipe, m_config);
    return true;
}

void IPCClient::Disconnect() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_connection) {
        m_connection->Close();
        m_connection.reset();
    }
}

bool IPCClient::IsConnected() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connection && m_connection->IsConnected();
}

IPCStatus IPCClient::Send(const MessageHeader& header, const void* payload) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_connection || !m_connection->IsConnected()) {
        return IPCStatus::CONNECTION_CLOSED;
    }
    return m_connection->Send(header, payload);
}

IPCStatus IPCClient::Receive(MessageHeader& header, std::vector<uint8_t>& payload, uint32_t timeoutMs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_connection || !m_connection->IsConnected()) {
        return IPCStatus::CONNECTION_CLOSED;
    }
    return m_connection->Receive(header, payload, timeoutMs == 0 ? m_config.readTimeoutMs : timeoutMs);
}

IPCStatus IPCClient::SendRequest(const MessageHeader& header, const void* payload,
                                 MessageHeader& responseHeader, std::vector<uint8_t>& responsePayload,
                                 uint32_t timeoutMs) {
    IPCStatus status = Send(header, payload);
    if (status != IPCStatus::OK) {
        if (m_autoReconnect && Reconnect(timeoutMs)) {
            status = Send(header, payload);
        }
        if (status != IPCStatus::OK) {
            return status;
        }
    }

    return Receive(responseHeader, responsePayload, timeoutMs == 0 ? m_config.readTimeoutMs : timeoutMs);
}

bool IPCClient::Reconnect(uint32_t timeoutMs) {
    Disconnect();
    return Connect(timeoutMs);
}

SharedMemoryRegion::SharedMemoryRegion(const std::wstring& name, uint32_t size, bool create)
    : m_mapping(nullptr), m_view(nullptr), m_size(size), m_name(name) {
    if (create) {
        m_mapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size, name.c_str());
    } else {
        m_mapping = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
    }

    if (m_mapping) {
        m_view = MapViewOfFile(m_mapping, FILE_MAP_ALL_ACCESS, 0, 0, size);
        if (!m_view) {
            CloseHandle(m_mapping);
            m_mapping = nullptr;
        }
    }
}

SharedMemoryRegion::~SharedMemoryRegion() {
    if (m_view) {
        UnmapViewOfFile(m_view);
        m_view = nullptr;
    }
    if (m_mapping) {
        CloseHandle(m_mapping);
        m_mapping = nullptr;
    }
}

bool SharedMemoryRegion::Write(const void* data, uint32_t size, uint32_t offset) {
    if (!m_view || !data || offset + size > m_size) {
        return false;
    }
    memcpy(static_cast<uint8_t*>(m_view) + offset, data, size);
    return true;
}

bool SharedMemoryRegion::Read(void* data, uint32_t size, uint32_t offset) {
    if (!m_view || !data || offset + size > m_size) {
        return false;
    }
    memcpy(data, static_cast<const uint8_t*>(m_view) + offset, size);
    return true;
}

ConnectionPool::ConnectionPool(const TransportConfig& config, uint32_t poolSize)
    : m_config(config), m_poolSize(poolSize) {
}

ConnectionPool::~ConnectionPool() {
    Drain();
}

bool ConnectionPool::Prewarm() {
    std::lock_guard<std::mutex> lock(m_mutex);
    while (m_available.size() < m_poolSize) {
        IPCClient client(m_config);
        if (!client.Connect(m_config.connectTimeoutMs)) {
            return false;
        }
        auto statusConn = std::make_unique<NamedPipeConnection>(
            CreateFileW(m_config.pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr),
            m_config);
        if (!statusConn->IsConnected()) {
            return false;
        }
        m_available.push_back(std::move(statusConn));
    }
    return true;
}

std::unique_ptr<IPCConnection> ConnectionPool::Acquire(uint32_t timeoutMs) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_available.empty()) {
        if (timeoutMs > 0) {
            m_cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() { return !m_available.empty(); });
        }
    }

    if (m_available.empty()) {
        return nullptr;
    }

    auto conn = std::move(m_available.back());
    m_available.pop_back();
    ++m_activeCount;
    return conn;
}

void ConnectionPool::Release(std::unique_ptr<IPCConnection> connection) {
    if (!connection) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_available.push_back(std::move(connection));
    if (m_activeCount > 0) {
        --m_activeCount;
    }
    m_cv.notify_one();
}

void ConnectionPool::Drain() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& conn : m_available) {
        if (conn) {
            conn->Close();
        }
    }
    m_available.clear();
    m_activeCount = 0;
}

uint32_t ConnectionPool::GetAvailableCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<uint32_t>(m_available.size());
}

uint32_t ConnectionPool::GetActiveCount() const {
    return m_activeCount.load();
}

} // namespace IPC
} // namespace DarkThumbs
