#pragma once
// =============================================================================
// CommandLineInterface.h — CLI Argument Parser for LENSManager
// ExplorerLens Engine — Utils Module
// =============================================================================

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ExplorerLens {

/// Argument value type
enum class ArgType : uint32_t {
    Flag = 0,      ///< Boolean flag (no value)
    String = 1,    ///< String value
    Integer = 2,   ///< Integer value
    FilePath = 3,  ///< File path (validated)
    Enum = 4       ///< Enumerated value from allowed set
};

/// Parsed argument result
struct ParsedArg
{
    std::wstring name;
    std::wstring value;
    ArgType type = ArgType::Flag;
    bool isPresent = false;
};

/// Argument definition for registration
struct ArgDefinition
{
    std::wstring longName;     ///< e.g. L"--output"
    std::wstring shortName;    ///< e.g. L"-o"
    std::wstring description;  ///< Help text
    ArgType type = ArgType::Flag;
    bool required = false;
    std::wstring defaultValue;
    std::vector<std::wstring> allowedValues;  ///< For ArgType::Enum
};

/// Parse result status
enum class ParseStatus : uint32_t {
    Success = 0,
    UnknownArgument = 1,
    MissingValue = 2,
    MissingRequired = 3,
    InvalidValue = 4,
    HelpRequested = 5
};

/// CommandLineInterface — parses and validates CLI arguments
class CommandLineInterface
{
  public:
    CommandLineInterface();
    explicit CommandLineInterface(const std::wstring& appName);

    // Registration
    void AddArgument(const ArgDefinition& def);
    void SetDescription(const std::wstring& desc);

    // Parsing
    ParseStatus Parse(int argc, const wchar_t* argv[]);
    ParseStatus Parse(const std::vector<std::wstring>& args);

    // Query results
    bool HasArg(const std::wstring& name) const;
    std::wstring GetString(const std::wstring& name, const std::wstring& defaultVal = L"") const;
    int GetInt(const std::wstring& name, int defaultVal = 0) const;
    bool GetFlag(const std::wstring& name) const;

    // Help
    std::wstring GenerateHelp() const;
    std::wstring GetErrorMessage() const
    {
        return m_errorMessage;
    }

    // State
    uint32_t GetArgumentCount() const
    {
        return static_cast<uint32_t>(m_definitions.size());
    }
    uint32_t GetParsedCount() const
    {
        return static_cast<uint32_t>(m_parsed.size());
    }
    const std::wstring& GetAppName() const
    {
        return m_appName;
    }

    // Static
    static const wchar_t* GetArgTypeName(ArgType type);
    static const wchar_t* GetParseStatusName(ParseStatus status);

  private:
    std::wstring m_appName;
    std::wstring m_description;
    std::vector<ArgDefinition> m_definitions;
    std::map<std::wstring, ParsedArg> m_parsed;
    std::wstring m_errorMessage;

    const ArgDefinition* FindDefinition(const std::wstring& name) const;
    std::wstring NormalizeName(const std::wstring& name) const;
};

}  // namespace ExplorerLens
