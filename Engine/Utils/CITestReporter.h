// CITestReporter.h — Test result reporting for CI/CD pipelines
// Copyright (c) 2026 ExplorerLens Project
//
// Collects test results and exports to JUnit XML (GitHub Actions),
// JSON (dashboards), and colored console summary. Tracks per-test timing.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

// ── Failure categorization ─────────────────────────────────────

enum class CIFailureCategory : uint8_t {
    Assertion,
    Timeout,
    Crash,
    ResourceExhaustion,
    EnvironmentIssue,
    Unknown
};

inline const char* CIFailureCategoryToString(CIFailureCategory cat) {
    switch (cat) {
        case CIFailureCategory::Assertion:          return "Assertion";
        case CIFailureCategory::Timeout:            return "Timeout";
        case CIFailureCategory::Crash:              return "Crash";
        case CIFailureCategory::ResourceExhaustion: return "ResourceExhaustion";
        case CIFailureCategory::EnvironmentIssue:   return "EnvironmentIssue";
        default:                                    return "Unknown";
    }
}

// ── Test result ────────────────────────────────────────────────

struct CITestEntry {
    std::string name;
    bool passed = true;
    double durationMs = 0.0;
    std::string failureMessage;
    CIFailureCategory failureCategory = CIFailureCategory::Unknown;
    std::string suiteName;
};

// ── CI Test Reporter ───────────────────────────────────────────

class CITestReporter {
public:
    CITestReporter() = default;

    explicit CITestReporter(const std::string& suiteName)
        : m_suiteName(suiteName)
    {
        m_startTime = std::chrono::steady_clock::now();
    }

    // Record a test result
    void AddResult(const CITestEntry& result) {
        m_results.push_back(result);
    }

    void AddResult(const std::string& name, bool passed, double durationMs = 0.0,
                   const std::string& failureMsg = "",
                   CIFailureCategory category = CIFailureCategory::Unknown) {
        CITestEntry r;
        r.name = name;
        r.passed = passed;
        r.durationMs = durationMs;
        r.failureMessage = failureMsg;
        r.failureCategory = category;
        r.suiteName = m_suiteName;
        m_results.push_back(r);
    }

    // Finalize timing
    void Finish() {
        m_endTime = std::chrono::steady_clock::now();
    }

    // ── Queries ────────────────────────────────────────────────

    size_t TotalTests() const { return m_results.size(); }

    size_t PassedTests() const {
        size_t count = 0;
        for (const auto& r : m_results) {
            if (r.passed) ++count;
        }
        return count;
    }

    size_t FailedTests() const {
        return TotalTests() - PassedTests();
    }

    double TotalDurationMs() const {
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            m_endTime - m_startTime).count();
        return static_cast<double>(elapsed) / 1000.0;
    }

    const std::vector<CITestEntry>& Results() const { return m_results; }

    // ── Export to JUnit XML (GitHub Actions) ───────────────────

    std::string ExportJUnitXML() const {
        std::ostringstream xml;
        xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        xml << "<testsuites tests=\"" << TotalTests()
            << "\" failures=\"" << FailedTests()
            << "\" time=\"" << std::fixed << std::setprecision(3)
            << (TotalDurationMs() / 1000.0) << "\">\n";

        xml << "  <testsuite name=\"" << EscapeXML(m_suiteName)
            << "\" tests=\"" << TotalTests()
            << "\" failures=\"" << FailedTests()
            << "\" time=\"" << std::fixed << std::setprecision(3)
            << (TotalDurationMs() / 1000.0) << "\">\n";

        for (const auto& r : m_results) {
            xml << "    <testcase name=\"" << EscapeXML(r.name)
                << "\" classname=\"" << EscapeXML(r.suiteName.empty() ? m_suiteName : r.suiteName)
                << "\" time=\"" << std::fixed << std::setprecision(3)
                << (r.durationMs / 1000.0) << "\"";

            if (r.passed) {
                xml << "/>\n";
            } else {
                xml << ">\n";
                xml << "      <failure message=\"" << EscapeXML(r.failureMessage)
                    << "\" type=\"" << CIFailureCategoryToString(r.failureCategory)
                    << "\"/>\n";
                xml << "    </testcase>\n";
            }
        }

        xml << "  </testsuite>\n";
        xml << "</testsuites>\n";
        return xml.str();
    }

    bool WriteJUnitXML(const std::string& filePath) const {
        std::ofstream ofs(filePath);
        if (!ofs.is_open()) return false;
        ofs << ExportJUnitXML();
        return ofs.good();
    }

    // ── Export to JSON ─────────────────────────────────────────

    std::string ExportJSON() const {
        std::ostringstream json;
        json << "{\n";
        json << "  \"suite\": \"" << EscapeJSON(m_suiteName) << "\",\n";
        json << "  \"totalTests\": " << TotalTests() << ",\n";
        json << "  \"passed\": " << PassedTests() << ",\n";
        json << "  \"failed\": " << FailedTests() << ",\n";
        json << "  \"totalDurationMs\": " << std::fixed << std::setprecision(2)
             << TotalDurationMs() << ",\n";
        json << "  \"results\": [\n";

        for (size_t i = 0; i < m_results.size(); ++i) {
            const auto& r = m_results[i];
            json << "    {\n";
            json << "      \"name\": \"" << EscapeJSON(r.name) << "\",\n";
            json << "      \"passed\": " << (r.passed ? "true" : "false") << ",\n";
            json << "      \"durationMs\": " << std::fixed << std::setprecision(2)
                 << r.durationMs << ",\n";
            json << "      \"failureMessage\": \"" << EscapeJSON(r.failureMessage) << "\",\n";
            json << "      \"failureCategory\": \"" << CIFailureCategoryToString(r.failureCategory) << "\"\n";
            json << "    }";
            if (i + 1 < m_results.size()) json << ",";
            json << "\n";
        }

        json << "  ]\n";
        json << "}\n";
        return json.str();
    }

    bool WriteJSON(const std::string& filePath) const {
        std::ofstream ofs(filePath);
        if (!ofs.is_open()) return false;
        ofs << ExportJSON();
        return ofs.good();
    }

    // ── Console summary (with ANSI color codes) ───────────────

    std::string ExportConsoleSummary() const {
        std::ostringstream out;
        constexpr const char* GREEN  = "\033[32m";
        constexpr const char* RED    = "\033[31m";
        constexpr const char* YELLOW = "\033[33m";
        constexpr const char* RESET  = "\033[0m";
        constexpr const char* BOLD   = "\033[1m";

        out << "\n" << BOLD << "======== Test Report: " << m_suiteName
            << " ========" << RESET << "\n";

        for (const auto& r : m_results) {
            if (r.passed) {
                out << GREEN << "  PASS" << RESET;
            } else {
                out << RED << "  FAIL" << RESET;
            }
            out << "  " << r.name;
            out << "  (" << std::fixed << std::setprecision(1) << r.durationMs << " ms)";
            if (!r.passed && !r.failureMessage.empty()) {
                out << "\n" << YELLOW << "       -> " << r.failureMessage << RESET;
            }
            out << "\n";
        }

        out << "\n" << BOLD << "Summary: " << RESET;
        out << PassedTests() << " passed, ";
        if (FailedTests() > 0)
            out << RED << FailedTests() << " failed" << RESET;
        else
            out << GREEN << "0 failed" << RESET;
        out << " (" << std::fixed << std::setprecision(1) << TotalDurationMs() << " ms total)\n";

        return out.str();
    }

private:
    std::string m_suiteName = "ExplorerLens";
    std::vector<CITestEntry> m_results;
    std::chrono::steady_clock::time_point m_startTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point m_endTime = std::chrono::steady_clock::now();

    // ── Helpers ────────────────────────────────────────────────

    static std::string EscapeXML(const std::string& input) {
        std::string out;
        out.reserve(input.size());
        for (char ch : input) {
            switch (ch) {
                case '&':  out += "&amp;";  break;
                case '<':  out += "&lt;";   break;
                case '>':  out += "&gt;";   break;
                case '"':  out += "&quot;"; break;
                case '\'': out += "&apos;"; break;
                default:   out += ch;       break;
            }
        }
        return out;
    }

    static std::string EscapeJSON(const std::string& input) {
        std::string out;
        out.reserve(input.size());
        for (char ch : input) {
            switch (ch) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:   out += ch;     break;
            }
        }
        return out;
    }
};

}} // namespace ExplorerLens::Engine
