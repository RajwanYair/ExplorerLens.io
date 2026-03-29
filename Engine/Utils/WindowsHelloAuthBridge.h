// WindowsHelloAuthBridge.h — Windows Hello Authentication Bridge
// Copyright (c) 2026 ExplorerLens Project
//
// Provides an abstraction layer for Windows Hello biometric and PIN authentication,
// enabling enterprise features such as locked annotation access and secure settings to
// require Windows Hello verification before granting access.
//
#pragma once
#include <string>
#include <functional>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

enum class HelloAuthMethod    { Fingerprint, FaceRecognition, PIN, None };
enum class HelloAuthStatus    { Approved, Denied, Cancelled, NotEnrolled, Unavailable };
enum class HelloProtectedScope { Annotations, PrivateSettings, PluginAdmin };

struct HelloAuthRequest {
    HelloProtectedScope scope       = HelloProtectedScope::Annotations;
    std::string         promptText  = "Authenticate to access this feature";
    bool                allowFallback = true;
};

struct HelloAuthResult {
    HelloAuthStatus  status    = HelloAuthStatus::Unavailable;
    HelloAuthMethod  method    = HelloAuthMethod::None;
    std::string      errorMsg;
    bool Ok() const noexcept { return status == HelloAuthStatus::Approved; }

    std::string StatusName() const noexcept {
        switch (status) {
        case HelloAuthStatus::Approved:    return "Approved";
        case HelloAuthStatus::Denied:      return "Denied";
        case HelloAuthStatus::Cancelled:   return "Cancelled";
        case HelloAuthStatus::NotEnrolled: return "NotEnrolled";
        case HelloAuthStatus::Unavailable: return "Unavailable";
        }
        return "Unknown";
    }
};

using HelloAuthFn = std::function<HelloAuthResult(const HelloAuthRequest&)>;

class WindowsHelloAuthBridge {
public:
    explicit WindowsHelloAuthBridge() = default;
    void SetAuthFunction(HelloAuthFn fn) { m_authFn = std::move(fn); }
    void SetAvailable(bool available) noexcept { m_available = available; }
    bool IsAvailable() const noexcept { return m_available; }

    HelloAuthResult Authenticate(const HelloAuthRequest& req) const {
        if (!m_available)
            return { HelloAuthStatus::Unavailable, HelloAuthMethod::None, "Windows Hello not available" };
        if (!m_authFn)
            return { HelloAuthStatus::Unavailable, HelloAuthMethod::None, "No auth function configured" };
        return m_authFn(req);
    }

    static std::string MethodName(HelloAuthMethod m) noexcept {
        switch (m) {
        case HelloAuthMethod::Fingerprint:     return "Fingerprint";
        case HelloAuthMethod::FaceRecognition: return "FaceRecognition";
        case HelloAuthMethod::PIN:             return "PIN";
        case HelloAuthMethod::None:            return "None";
        }
        return "Unknown";
    }

    static std::string ScopeName(HelloProtectedScope s) noexcept {
        switch (s) {
        case HelloProtectedScope::Annotations:    return "Annotations";
        case HelloProtectedScope::PrivateSettings: return "PrivateSettings";
        case HelloProtectedScope::PluginAdmin:    return "PluginAdmin";
        }
        return "Unknown";
    }

private:
    bool         m_available = false;
    HelloAuthFn  m_authFn;
};

} // namespace Engine
} // namespace ExplorerLens
