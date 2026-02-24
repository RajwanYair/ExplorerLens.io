#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include "../Engine/contracts/thumbnail_contracts.h"

namespace ExplorerLens::Service {

    enum class ServiceTransport {
        NamedPipe,
        HttpLocalhost
    };

    struct ServiceConfig {
        ServiceTransport transport;
        uint16_t port;              // For HTTP
        std::wstring pipeName;      // For NamedPipe
        bool requireAuthToken;
    };

    // Abstract handler for service requests
    class IRequestHandler {
    public:
        virtual ~IRequestHandler() = default;
        // Returns JSON string response
        virtual std::string HandleRequest(const std::string& method, const std::string& path, const std::string& body) = 0;
    };

    class LocalApiService {
    public:
        LocalApiService(const ServiceConfig& config);
        virtual ~LocalApiService();

        // Start the listener loop (blocking or threaded)
        void Start();
        
        // Stop the service
        void Stop();

        // Register handlers for specific routes
        // e.g. RegisterRoute("GET", "/v1/status", ...)
        void RegisterRoute(const std::string& method, const std::string& path, std::function<std::string()> handler);

    private:
        ServiceConfig m_config;
        bool m_isRunning;
        std::unique_ptr<IRequestHandler> m_handler;
        std::map<std::string, std::function<std::string()>> m_routes;
        std::unique_ptr<std::thread> m_listenerThread;
        std::mutex m_routeMutex;
        
        void ListenHttp();
        void ListenPipe();
    };

}

