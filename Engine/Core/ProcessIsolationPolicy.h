// ProcessIsolationPolicy.h — Per-Format Process Isolation Policy
// Copyright (c) 2026 ExplorerLens Project
//
// Defines and enforces per-file-format sandbox isolation levels for the
// out-of-process thumbnail engine. Archives, PDF, and untrusted formats
// run at High isolation; well-known raster formats run at Low. Rules are
// evaluated at process-spawn time so the COM surrogate receives the least-
// privilege token appropriate for the format being decoded.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class IsolationLevel : uint8_t {
    None = 0,
    Low = 1,
    Medium = 2,
    High = 3,
    Strict = 4,
};

enum class IsolationAction : uint8_t {
    Allow = 0,
    Deny = 1,
    Sandbox = 2,
};

struct FormatIsolationRule
{
    std::wstring formatExt;
    IsolationLevel level = IsolationLevel::Medium;
    IsolationAction action = IsolationAction::Sandbox;
};

struct PolicyResult
{
    bool allowed = true;
    IsolationLevel effectiveLevel = IsolationLevel::Medium;
    IsolationAction action = IsolationAction::Allow;
    std::string reason;
};

class ProcessIsolationPolicy
{
  public:
    static constexpr IsolationLevel DEFAULT_LEVEL = IsolationLevel::Medium;
    static constexpr IsolationLevel ARCHIVE_LEVEL = IsolationLevel::High;
    static constexpr IsolationLevel SAFE_LEVEL = IsolationLevel::Low;

    ProcessIsolationPolicy() noexcept
    {
        LoadBuiltinRules();
    }

    [[nodiscard]] IsolationLevel GetDefaultLevel() const noexcept
    {
        return m_defaultLevel;
    }

    void SetDefaultLevel(IsolationLevel level) noexcept
    {
        m_defaultLevel = level;
    }

    void AddRule(const FormatIsolationRule& rule) noexcept
    {
        for (auto& r : m_rules) {
            if (r.formatExt == rule.formatExt) {
                r = rule;
                return;
            }
        }
        m_rules.push_back(rule);
    }

    void ClearRules() noexcept
    {
        m_rules.clear();
    }

    [[nodiscard]] PolicyResult Evaluate(const std::wstring& filePath) const noexcept
    {
        PolicyResult result;
        auto ext = ExtractExtension(filePath);
        auto it = FindRule(ext);
        if (it != m_rules.end()) {
            result.effectiveLevel = it->level;
            result.action = it->action;
            result.allowed = (it->action != IsolationAction::Deny);
            result.reason = "format rule";
        } else {
            result.effectiveLevel = m_defaultLevel;
            result.action = IsolationAction::Allow;
            result.allowed = true;
            result.reason = "default";
        }
        return result;
    }

    [[nodiscard]] IsolationLevel GetLevelForFormat(const std::wstring& ext) const noexcept
    {
        auto normalized = ToLower(ext);
        auto it = FindRule(normalized);
        return (it != m_rules.end()) ? it->level : m_defaultLevel;
    }

    [[nodiscard]] bool ShouldSandbox(const std::wstring& ext) const noexcept
    {
        auto it = FindRule(ToLower(ext));
        if (it != m_rules.end()) {
            return it->action == IsolationAction::Sandbox;
        }
        return m_defaultLevel >= IsolationLevel::High;
    }

    static const wchar_t* GetLevelName(IsolationLevel level) noexcept
    {
        switch (level) {
            case IsolationLevel::None:
                return L"None";
            case IsolationLevel::Low:
                return L"Low";
            case IsolationLevel::Medium:
                return L"Medium";
            case IsolationLevel::High:
                return L"High";
            case IsolationLevel::Strict:
                return L"Strict";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* GetActionName(IsolationAction action) noexcept
    {
        switch (action) {
            case IsolationAction::Allow:
                return L"Allow";
            case IsolationAction::Deny:
                return L"Deny";
            case IsolationAction::Sandbox:
                return L"Sandbox";
            default:
                return L"Unknown";
        }
    }

  private:
    IsolationLevel m_defaultLevel = DEFAULT_LEVEL;
    std::vector<FormatIsolationRule> m_rules;

    void LoadBuiltinRules() noexcept
    {
        struct BuiltinRule
        {
            const wchar_t* ext;
            IsolationLevel level;
            IsolationAction action;
        };
        static constexpr BuiltinRule kBuiltin[] = {
            {L".zip", IsolationLevel::High, IsolationAction::Sandbox},
            {L".rar", IsolationLevel::High, IsolationAction::Sandbox},
            {L".7z", IsolationLevel::High, IsolationAction::Sandbox},
            {L".tar", IsolationLevel::High, IsolationAction::Sandbox},
            {L".gz", IsolationLevel::Medium, IsolationAction::Sandbox},
            {L".pdf", IsolationLevel::High, IsolationAction::Sandbox},
            {L".png", IsolationLevel::Low, IsolationAction::Allow},
            {L".jpg", IsolationLevel::Low, IsolationAction::Allow},
            {L".jpeg", IsolationLevel::Low, IsolationAction::Allow},
            {L".bmp", IsolationLevel::Low, IsolationAction::Allow},
            {L".webp", IsolationLevel::Low, IsolationAction::Allow},
            {L".gif", IsolationLevel::Low, IsolationAction::Allow},
        };
        for (const auto& b : kBuiltin) {
            m_rules.push_back({b.ext, b.level, b.action});
        }
    }

    static std::wstring ExtractExtension(const std::wstring& path) noexcept
    {
        auto dot = path.rfind(L'.');
        if (dot == std::wstring::npos)
            return L"";
        return ToLower(path.substr(dot));
    }

    static std::wstring ToLower(const std::wstring& s) noexcept
    {
        std::wstring r = s;
        for (auto& c : r) {
            if (c >= L'A' && c <= L'Z')
                c = c + (L'a' - L'A');
        }
        return r;
    }

    std::vector<FormatIsolationRule>::const_iterator FindRule(const std::wstring& ext) const noexcept
    {
        for (auto it = m_rules.begin(); it != m_rules.end(); ++it) {
            if (it->formatExt == ext)
                return it;
        }
        return m_rules.end();
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
