// =============================================================================
// ipc_transport.h - IPC Transport Layer (Named Pipes + Shared Memory)
// =============================================================================
// Cross-platform transport abstraction for IPC communication
// =============================================================================

#pragma once

#include "ipc_protocol.h"
#include <windows.h>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace ExplorerLens {
namespace IPC {

// =============================================================================
// Forward Declarations
// =============================================================================

class IPCConnection;
class IPCServer;
class IPCClient;

// =============================================================================
// IPC Connection (Base class for client/server sides)
// =============================================================================

class IPCConnection {
public:
    virtual ~IPCConnection() = default;
    
    // Send/Receive Messages
    virtual IPCStatus Send(const MessageHeader& header, const void* payload = nullptr) = 0;
    virtual IPCStatus Receive(MessageHeader& header, std::vector<uint8_t>& payload, uint32_t timeoutMs = 0) = 0;
    
    // Connection Management
    virtual bool IsConnected() const = 0;
    virtual void Close() = 0;
    
    // Statistics
    virtual uint64_t GetBytesSent() const = 0;
    virtual uint64_t GetBytesReceived() const = 0;
    virtual uint32_t GetMessagesSent() const = 0;
    virtual uint32_t GetMessagesReceived() const = 0;
};

// =============================================================================
// Named Pipe Connection
// =============================================================================

class NamedPipeConnection : public IPCConnection {
public:
    NamedPipeConnection(HANDLE pipe, const TransportConfig& config);
    ~NamedPipeConnection() override;
    
    // IPCConnection implementation
    IPCStatus Send(const MessageHeader& header, const void* payload = nullptr) override;
    IPCStatus Receive(MessageHeader& header, std::vector<uint8_t>& payload, uint32_t timeoutMs = 0) override;
    bool IsConnected() const override;
    void Close() override;
    
    uint64_t GetBytesSent() const override { return m_bytesSent; }
    uint64_t GetBytesReceived() const override { return m_bytesReceived; }
    uint32_t GetMessagesSent() const override { return m_messagesSent; }
    uint32_t GetMessagesReceived() const override { return m_messagesReceived; }
    
    // Named Pipe specific
    HANDLE GetHandle() const { return m_pipe; }
    
private:
    HANDLE m_pipe;
    TransportConfig m_config;
    mutable std::mutex m_mutex;
    
    std::atomic<uint64_t> m_bytesSent{0};
    std::atomic<uint64_t> m_bytesReceived{0};
    std::atomic<uint32_t> m_messagesSent{0};
    std::atomic<uint32_t> m_messagesReceived{0};
    
    IPCStatus WriteData(const void* data, uint32_t size);
    IPCStatus ReadData(void* data, uint32_t size, uint32_t timeoutMs);
};

// =============================================================================
// IPC Server (Worker side - listens for connections)
// =============================================================================

class IPCServer {
public:
    using ConnectionCallback = std::function<void(std::unique_ptr<IPCConnection>)>;
    
    explicit IPCServer(const TransportConfig& config);
    ~IPCServer();
    
    // Start/Stop Server
    bool Start();
    void Stop();
    bool IsRunning() const { return m_running; }
    
    // Accept Connections
    std::unique_ptr<IPCConnection> Accept(uint32_t timeoutMs = INFINITE);
    
    // Async Connection Handling
    void SetConnectionCallback(ConnectionCallback callback);
    bool StartAsync(); // Start accepting connections in background
    
    const std::wstring& GetPipeName() const { return m_pipeName; }
    
private:
    TransportConfig m_config;
    std::wstring m_pipeName;
    HANDLE m_pipeHandle;
    std::atomic<bool> m_running{false};
    ConnectionCallback m_connectionCallback;
    
    HANDLE CreatePipeInstance();
};

// =============================================================================
// IPC Client (ShellHost side - connects to server)
// =============================================================================

class IPCClient {
public:
    explicit IPCClient(const TransportConfig& config);
    ~IPCClient();
    
    // Connection Management
    bool Connect(uint32_t timeoutMs = 0);
    void Disconnect();
    bool IsConnected() const;
    
    // Send/Receive
    IPCStatus Send(const MessageHeader& header, const void* payload = nullptr);
    IPCStatus Receive(MessageHeader& header, std::vector<uint8_t>& payload, uint32_t timeoutMs = 0);
    
    // Request/Response Pattern
    IPCStatus SendRequest(const MessageHeader& header, const void* payload,
                          MessageHeader& responseHeader, std::vector<uint8_t>& responsePayload,
                          uint32_t timeoutMs = 0);
    
    // Connection Statistics
    uint64_t GetBytesSent() const { return m_connection ? m_connection->GetBytesSent() : 0; }
    uint64_t GetBytesReceived() const { return m_connection ? m_connection->GetBytesReceived() : 0; }
    
    // Reconnection
    bool Reconnect(uint32_t timeoutMs = 0);
    void SetAutoReconnect(bool enable) { m_autoReconnect = enable; }
    
private:
    TransportConfig m_config;
    std::wstring m_pipeName;
    std::unique_ptr<IPCConnection> m_connection;
    std::atomic<bool> m_autoReconnect{false};
    mutable std::mutex m_mutex;
    
    HANDLE ConnectToPipe(uint32_t timeoutMs);
};

// =============================================================================
// Shared Memory Helper (for large payload optimization)
// =============================================================================

class SharedMemoryRegion {
public:
    SharedMemoryRegion(const std::wstring& name, uint32_t size, bool create);
    ~SharedMemoryRegion();
    
    bool IsValid() const { return m_mapping != nullptr && m_view != nullptr; }
    void* GetView() { return m_view; }
    uint32_t GetSize() const { return m_size; }
    
    // Write/Read helpers
    bool Write(const void* data, uint32_t size, uint32_t offset = 0);
    bool Read(void* data, uint32_t size, uint32_t offset = 0);
    
private:
    HANDLE m_mapping;
    void* m_view;
    uint32_t m_size;
    std::wstring m_name;
};

// =============================================================================
// IPC Exception for error handling
// =============================================================================

class IPCException : public std::exception {
public:
    IPCException(IPCStatus status, const std::string& message)
        : m_status(status), m_message(message) {}
    
    const char* what() const noexcept override { return m_message.c_str(); }
    IPCStatus GetStatus() const { return m_status; }
    
private:
    IPCStatus m_status;
    std::string m_message;
};

// =============================================================================
// Utility Functions
// =============================================================================

inline std::wstring GenerateUniquePipeName(const std::wstring& prefix) {
    return prefix + std::to_wstring(GetCurrentProcessId());
}

inline bool WaitForPipe(const std::wstring& pipeName, uint32_t timeoutMs) {
    return WaitNamedPipeW(pipeName.c_str(), timeoutMs) != 0;
}

// =============================================================================
// Connection Pool (for warm worker management)
// =============================================================================

class ConnectionPool {
public:
    explicit ConnectionPool(const TransportConfig& config, uint32_t poolSize);
    ~ConnectionPool();
    
    // Get/Return connections
    std::unique_ptr<IPCConnection> Acquire(uint32_t timeoutMs = 0);
    void Release(std::unique_ptr<IPCConnection> connection);
    
    // Pool management
    bool Prewarm();
    void Drain();
    uint32_t GetAvailableCount() const;
    uint32_t GetActiveCount() const;
    
private:
    TransportConfig m_config;
    uint32_t m_poolSize;
    std::vector<std::unique_ptr<IPCConnection>> m_available;
    std::atomic<uint32_t> m_activeCount{0};
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
};

} // namespace IPC
} // namespace ExplorerLens

