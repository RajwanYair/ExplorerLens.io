//==============================================================================
// NetworkProviderEngine
//==============================================================================

#include "NetworkProviderEngine.h"
#include <chrono>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

NetworkProviderEngine::NetworkProviderEngine() {}

NetworkPath NetworkProviderEngine::ParsePath(const std::wstring& path) const {
    NetworkPath result;
    result.fullPath = path;
    result.protocol = DetectProtocol(path);

    if (path.size() >= 2 && path[0] == L'\\' && path[1] == L'\\') {
        // UNC path: \\server\share\path
        auto serverStart = 2;
        auto serverEnd = path.find(L'\\', serverStart);
        if (serverEnd != std::wstring::npos) {
            result.server = path.substr(serverStart, serverEnd - serverStart);
            auto shareEnd = path.find(L'\\', serverEnd + 1);
            if (shareEnd != std::wstring::npos) {
                result.share = path.substr(serverEnd + 1, shareEnd - serverEnd - 1);
                result.relativePath = path.substr(shareEnd + 1);
            } else {
                result.share = path.substr(serverEnd + 1);
            }
        }
    }

    return result;
}

NetworkThumbnailResult NetworkProviderEngine::FetchThumbnail(
    const std::wstring& networkPath)
{
    NetworkThumbnailResult result;
    auto start = std::chrono::high_resolution_clock::now();

    auto parsed = ParsePath(networkPath);
    if (parsed.server.empty()) {
        result.status = NetworkStatus::Error;
        result.errorMessage = L"Invalid network path";
        return result;
    }

    result.status = CheckConnectivity(parsed.server);

    auto end = std::chrono::high_resolution_clock::now();
    result.latencyMs = std::chrono::duration<double, std::milli>(end - start).count();

    if (result.status == NetworkStatus::Connected) {
        result.success = true;
    }

    return result;
}

NetworkStatus NetworkProviderEngine::CheckConnectivity(
    const std::wstring& /*server*/) const
{
    // In production: ping or try to connect to server
    return NetworkStatus::Connected;
}

bool NetworkProviderEngine::IsNetworkPath(const std::wstring& path) {
    if (path.size() < 2) return false;
    // UNC
    if (path[0] == L'\\' && path[1] == L'\\') return true;
    // URL-based
    if (path.find(L"://") != std::wstring::npos) return true;
    return false;
}

NetworkProtocol NetworkProviderEngine::DetectProtocol(const std::wstring& path) {
    if (path.find(L"ftp://") == 0 || path.find(L"FTP://") == 0)
        return NetworkProtocol::FTP;
    if (path.find(L"sftp://") == 0 || path.find(L"SFTP://") == 0)
        return NetworkProtocol::SFTP;
    if (path.find(L"http://") == 0 || path.find(L"https://") == 0)
        return NetworkProtocol::HTTP;
    if (path.find(L"webdav://") == 0 || path.find(L"dav://") == 0)
        return NetworkProtocol::WebDAV;
    if (path.size() >= 2 && path[0] == L'\\' && path[1] == L'\\')
        return NetworkProtocol::UNC;
    return NetworkProtocol::SMB;
}

const wchar_t* NetworkProviderEngine::GetProtocolName(NetworkProtocol protocol) {
    switch (protocol) {
        case NetworkProtocol::UNC:    return L"UNC";
        case NetworkProtocol::SMB:    return L"SMB";
        case NetworkProtocol::WebDAV: return L"WebDAV";
        case NetworkProtocol::FTP:    return L"FTP";
        case NetworkProtocol::SFTP:   return L"SFTP";
        case NetworkProtocol::HTTP:   return L"HTTP";
        default: return L"Unknown";
    }
}

const wchar_t* NetworkProviderEngine::GetStatusName(NetworkStatus status) {
    switch (status) {
        case NetworkStatus::Connected:    return L"Connected";
        case NetworkStatus::Disconnected: return L"Disconnected";
        case NetworkStatus::Timeout:      return L"Timeout";
        case NetworkStatus::AuthRequired: return L"Auth Required";
        case NetworkStatus::Error:        return L"Error";
        default: return L"Unknown";
    }
}

}} // namespace ExplorerLens::Engine

