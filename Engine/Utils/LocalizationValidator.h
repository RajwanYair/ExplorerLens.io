// LocalizationValidator.h — String Length and RTL/LTR Layout Verifier
// Copyright (c) 2026 ExplorerLens Project
//
// Validates all translated string resources for:
//   - Length overruns vs UI control size
//   - RTL (Arabic, Hebrew) layout correctness
//   - Missing placeholders vs source string
//   - Forbidden characters for each locale
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class LocaleDirection : uint8_t {
    LTR = 0,
    RTL = 1,
};

struct LocaleProperties
{
    std::string lcid;  // e.g. "en-US", "ar-SA", "he-IL"
    LocaleDirection direction;
    uint32_t maxStringLengthChars{256};  // UI hard cap
    bool requiresBidiMirror{false};
};

struct ValidationIssue
{
    std::string lcid;
    uint32_t stringId;
    std::string key;
    std::string sourceText;
    std::string translatedText;
    std::string issueDescription;

    enum class Severity {
        Warning,
        Error
    } severity{Severity::Warning};
};

struct ValidationReport
{
    std::string timestamp;
    uint32_t totalStrings{0};
    uint32_t passed{0};
    uint32_t warnings{0};
    uint32_t errors{0};
    std::vector<ValidationIssue> issues;
    bool overallPassed{false};
};

class LocalizationValidator
{
  public:
    struct Config
    {
        std::string resourceDir;  // path to .rc files
        std::string sourceLocale{"en-US"};
        std::vector<std::string> targetLocales;
        uint32_t maxLengthRatioPct{130};  // translated text <= 130% of source length
        bool strictPlaceholderCheck{true};
    };

    explicit LocalizationValidator(Config cfg);

    // Run validation across all configured locales.
    [[nodiscard]] ValidationReport Validate() const;

    // Validate a single locale.
    [[nodiscard]] ValidationReport ValidateLocale(const std::string& lcid) const;

    // Check if a string requires RTL BiDi embedding marks.
    static bool RequiresBidiMark(std::string_view text, LocaleDirection dir) noexcept;

    // Extract {0}, {1}, %s, %d placeholders from a string.
    static std::vector<std::string> ExtractPlaceholders(std::string_view text);

    // Check placeholder parity between source and translation.
    static bool PlaceholdersMatch(std::string_view source, std::string_view translated);

    // Generate a validation summary JSON string.
    static std::string ToJson(const ValidationReport& report);

    static LocaleProperties GetLocaleInfo(const std::string& lcid) noexcept;

  private:
    Config m_cfg;
};

// Supported locales with RTL/LTR info
inline const LocaleProperties g_supportedLocales[] = {
    {"en-US", LocaleDirection::LTR, 256, false}, {"en-GB", LocaleDirection::LTR, 256, false},
    {"fr-FR", LocaleDirection::LTR, 280, false}, {"de-DE", LocaleDirection::LTR, 300, false},
    {"ja-JP", LocaleDirection::LTR, 200, false}, {"zh-CN", LocaleDirection::LTR, 180, false},
    {"ko-KR", LocaleDirection::LTR, 200, false}, {"ar-SA", LocaleDirection::RTL, 256, true},
    {"he-IL", LocaleDirection::RTL, 256, true},
};

}  // namespace Engine
}  // namespace ExplorerLens
