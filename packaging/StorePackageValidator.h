// StorePackageValidator.h — Microsoft Store Package Validation
// Copyright (c) 2026 ExplorerLens Project
//
// Validates MSIX packages against Microsoft Store submission requirements:
// manifest schema, capability declarations, content policy rules, and
// packaging constraints for the ExplorerLens shell extension.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class StoreCheckSeverity { Pass, Warning, Failure };

struct StoreCheckResult {
    std::wstring       checkId;
    std::wstring       checkName;
    StoreCheckSeverity severity = StoreCheckSeverity::Pass;
    std::wstring       detail;
};

struct StoreValidationReport {
    std::vector<StoreCheckResult> results;
    int passCount    = 0;
    int warnCount    = 0;
    int failureCount = 0;
    bool IsSubmittable() const { return failureCount == 0; }
};

// Manifest metadata expected for Store submission
struct MSIXManifestMeta {
    std::wstring packageName;   // e.g. "RajwanYair.ExplorerLens"
    std::wstring publisherId;   // CN=... certificate subject
    std::wstring version;       // Major.Minor.Build.Revision (4 parts)
    std::wstring architecture;  // x64, arm64, neutral
    std::wstring minOsVersion;  // e.g. "10.0.22000.0"
    bool         hasPrivacyUrl  = false;
    bool         hasDescription = false;
    bool         hasSupportUrl  = false;
    std::vector<std::wstring> capabilities;
    std::vector<std::wstring> fileTypeAssociations;
};

class StorePackageValidator {
public:
    StoreValidationReport Validate(const MSIXManifestMeta& meta) {
        StoreValidationReport report;

        auto add = [&](const wchar_t* id, const wchar_t* name,
                       StoreCheckSeverity sev, const std::wstring& detail) {
            report.results.push_back({ id, name, sev, detail });
            if (sev == StoreCheckSeverity::Pass)    report.passCount++;
            else if (sev == StoreCheckSeverity::Warning) report.warnCount++;
            else report.failureCount++;
        };

        // STC-001: Package identity
        if (meta.packageName.empty())
            add(L"STC-001", L"Package Identity", StoreCheckSeverity::Failure,
                L"packageName must not be empty");
        else
            add(L"STC-001", L"Package Identity", StoreCheckSeverity::Pass, meta.packageName);

        // STC-002: Publisher ID format
        if (meta.publisherId.find(L"CN=") == std::wstring::npos)
            add(L"STC-002", L"Publisher Certificate", StoreCheckSeverity::Failure,
                L"publisherId must be a valid certificate distinguished name (CN=...)");
        else
            add(L"STC-002", L"Publisher Certificate", StoreCheckSeverity::Pass, meta.publisherId);

        // STC-003: Version 4-part format
        {
            int parts = 0;
            size_t pos = 0;
            while ((pos = meta.version.find(L'.', pos)) != std::wstring::npos) { parts++; pos++; }
            if (parts != 3)
                add(L"STC-003", L"Version Format", StoreCheckSeverity::Failure,
                    L"Version must be Major.Minor.Build.Revision (4 parts)");
            else
                add(L"STC-003", L"Version Format", StoreCheckSeverity::Pass, meta.version);
        }

        // STC-004: Minimum OS version
        if (meta.minOsVersion.empty() || meta.minOsVersion < L"10.0.17763.0")
            add(L"STC-004", L"Min OS Version", StoreCheckSeverity::Warning,
                L"Recommend minOsVersion >= 10.0.17763.0 (RS5) for Shell Extension support");
        else
            add(L"STC-004", L"Min OS Version", StoreCheckSeverity::Pass, meta.minOsVersion);

        // STC-005: Privacy policy URL required
        if (!meta.hasPrivacyUrl)
            add(L"STC-005", L"Privacy Policy", StoreCheckSeverity::Failure,
                L"Store requires a privacy policy URL");
        else
            add(L"STC-005", L"Privacy Policy", StoreCheckSeverity::Pass, L"Present");

        // STC-006: Description present
        if (!meta.hasDescription)
            add(L"STC-006", L"Product Description", StoreCheckSeverity::Warning,
                L"Add a StoreDescription element for Store listing");
        else
            add(L"STC-006", L"Product Description", StoreCheckSeverity::Pass, L"Present");

        // STC-007: Architecture
        if (meta.architecture != L"x64" && meta.architecture != L"arm64" &&
            meta.architecture != L"neutral")
            add(L"STC-007", L"Architecture", StoreCheckSeverity::Failure,
                L"Supported: x64, arm64, neutral");
        else
            add(L"STC-007", L"Architecture", StoreCheckSeverity::Pass, meta.architecture);

        // STC-008: No restricted capabilities without justification
        static const wchar_t* RESTRICTED[] = {
            L"broadFileSystemAccess", L"runFullTrust", L"allowElevation", nullptr
        };
        for (const auto& cap : meta.capabilities) {
            for (int i = 0; RESTRICTED[i]; ++i) {
                if (cap == RESTRICTED[i]) {
                    add(L"STC-008", L"Restricted Capabilities",
                        StoreCheckSeverity::Warning,
                        std::wstring(L"Requires Store justification: ") + cap);
                }
            }
        }
        if (report.results.back().checkId != L"STC-008")
            add(L"STC-008", L"Restricted Capabilities", StoreCheckSeverity::Pass,
                L"No unrestricted capabilities");

        // STC-009: Shell extension must declare runFullTrust
        bool hasFullTrust = false;
        for (const auto& cap : meta.capabilities)
            if (cap == L"runFullTrust") { hasFullTrust = true; break; }
        if (!hasFullTrust)
            add(L"STC-009", L"Shell Extension Trust", StoreCheckSeverity::Failure,
                L"IThumbnailProvider shell extensions require runFullTrust capability");
        else
            add(L"STC-009", L"Shell Extension Trust", StoreCheckSeverity::Pass,
                L"runFullTrust declared");

        // STC-010: Support URL
        if (!meta.hasSupportUrl)
            add(L"STC-010", L"Support URL", StoreCheckSeverity::Warning,
                L"Providing a support URL improves Store review outcomes");
        else
            add(L"STC-010", L"Support URL", StoreCheckSeverity::Pass, L"Present");

        return report;
    }

    // Render report as Markdown table
    static std::wstring ToMarkdown(const StoreValidationReport& r) {
        std::wstring md = L"# Store Package Validation Report\n\n";
        md += L"| Check | Name | Result | Detail |\n";
        md += L"|---|---|---|---|\n";
        for (const auto& c : r.results) {
            const wchar_t* icon = (c.severity == StoreCheckSeverity::Pass)    ? L"PASS"
                                : (c.severity == StoreCheckSeverity::Warning)  ? L"WARN"
                                                                                : L"FAIL";
            md += L"| " + c.checkId + L" | " + c.checkName + L" | " + icon +
                  L" | " + c.detail + L" |\n";
        }
        md += L"\n**Submittable:** " + std::wstring(r.IsSubmittable() ? L"Yes" : L"No") + L"\n";
        return md;
    }
};

}} // namespace ExplorerLens::Engine
