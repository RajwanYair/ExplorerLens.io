// =============================================================================
// CommandLineInterface.cpp — CLI Argument Parser for LENSManager
// ExplorerLens Engine — Utils Module
// =============================================================================

#include "CommandLineInterface.h"
#include <sstream>
#include <algorithm>

namespace ExplorerLens {

CommandLineInterface::CommandLineInterface() : m_appName(L"ExplorerLens") {}

CommandLineInterface::CommandLineInterface(const std::wstring& appName) : m_appName(appName) {}

void CommandLineInterface::AddArgument(const ArgDefinition& def) {
    m_definitions.push_back(def);
}

void CommandLineInterface::SetDescription(const std::wstring& desc) {
    m_description = desc;
}

std::wstring CommandLineInterface::NormalizeName(const std::wstring& name) const {
    // Strip leading dashes for lookup
    std::wstring n = name;
    while (!n.empty() && n[0] == L'-') n.erase(0, 1);
    return n;
}

const ArgDefinition* CommandLineInterface::FindDefinition(const std::wstring& name) const {
    for (const auto& def : m_definitions) {
        if (def.longName == name || def.shortName == name) return &def;
        // Also match normalized
        if (NormalizeName(def.longName) == NormalizeName(name)) return &def;
    }
    return nullptr;
}

ParseStatus CommandLineInterface::Parse(const std::vector<std::wstring>& args) {
    m_parsed.clear();
    m_errorMessage.clear();

    for (size_t i = 0; i < args.size(); ++i) {
        const auto& arg = args[i];

        // Check for help
        if (arg == L"--help" || arg == L"-h" || arg == L"-?" || arg == L"/?") {
            return ParseStatus::HelpRequested;
        }

        // Find matching definition
        const ArgDefinition* def = FindDefinition(arg);
        if (!def) {
            m_errorMessage = L"Unknown argument: " + arg;
            return ParseStatus::UnknownArgument;
        }

        ParsedArg parsed;
        parsed.name = def->longName;
        parsed.type = def->type;
        parsed.isPresent = true;

        if (def->type == ArgType::Flag) {
            parsed.value = L"true";
        } else {
            // Need a value
            if (i + 1 >= args.size()) {
                m_errorMessage = L"Missing value for: " + arg;
                return ParseStatus::MissingValue;
            }
            parsed.value = args[++i];

            // Validate enum values
            if (def->type == ArgType::Enum && !def->allowedValues.empty()) {
                bool found = false;
                for (const auto& av : def->allowedValues) {
                    if (av == parsed.value) { found = true; break; }
                }
                if (!found) {
                    m_errorMessage = L"Invalid value '" + parsed.value + L"' for: " + arg;
                    return ParseStatus::InvalidValue;
                }
            }
        }

        m_parsed[def->longName] = parsed;
    }

    // Check required arguments
    for (const auto& def : m_definitions) {
        if (def.required && m_parsed.find(def.longName) == m_parsed.end()) {
            m_errorMessage = L"Missing required argument: " + def.longName;
            return ParseStatus::MissingRequired;
        }
    }

    return ParseStatus::Success;
}

ParseStatus CommandLineInterface::Parse(int argc, const wchar_t* argv[]) {
    std::vector<std::wstring> args;
    // Skip argv[0] (program name)
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    return Parse(args);
}

bool CommandLineInterface::HasArg(const std::wstring& name) const {
    auto it = m_parsed.find(name);
    return it != m_parsed.end() && it->second.isPresent;
}

std::wstring CommandLineInterface::GetString(const std::wstring& name, const std::wstring& defaultVal) const {
    auto it = m_parsed.find(name);
    if (it != m_parsed.end() && it->second.isPresent) return it->second.value;
    // Check for default in definition
    const ArgDefinition* def = FindDefinition(name);
    if (def && !def->defaultValue.empty()) return def->defaultValue;
    return defaultVal;
}

int CommandLineInterface::GetInt(const std::wstring& name, int defaultVal) const {
    auto it = m_parsed.find(name);
    if (it != m_parsed.end() && it->second.isPresent) {
        try { return std::stoi(it->second.value); }
        catch (...) { return defaultVal; }
    }
    return defaultVal;
}

bool CommandLineInterface::GetFlag(const std::wstring& name) const {
    return HasArg(name);
}

std::wstring CommandLineInterface::GenerateHelp() const {
    std::wstringstream ss;
    ss << m_appName << L" — " << m_description << L"\n\n";
    ss << L"Usage: " << m_appName << L" [options]\n\n";
    ss << L"Options:\n";
    for (const auto& def : m_definitions) {
        ss << L"  " << def.shortName << L", " << def.longName;
        if (def.type != ArgType::Flag) ss << L" <" << GetArgTypeName(def.type) << L">";
        ss << L"\n        " << def.description;
        if (def.required) ss << L" (required)";
        if (!def.defaultValue.empty()) ss << L" [default: " << def.defaultValue << L"]";
        ss << L"\n";
    }
    return ss.str();
}

const wchar_t* CommandLineInterface::GetArgTypeName(ArgType type) {
    switch (type) {
        case ArgType::Flag:     return L"Flag";
        case ArgType::String:   return L"String";
        case ArgType::Integer:  return L"Integer";
        case ArgType::FilePath: return L"FilePath";
        case ArgType::Enum:     return L"Enum";
        default:                return L"Unknown";
    }
}

const wchar_t* CommandLineInterface::GetParseStatusName(ParseStatus status) {
    switch (status) {
        case ParseStatus::Success:          return L"Success";
        case ParseStatus::UnknownArgument:  return L"Unknown Argument";
        case ParseStatus::MissingValue:     return L"Missing Value";
        case ParseStatus::MissingRequired:  return L"Missing Required";
        case ParseStatus::InvalidValue:     return L"Invalid Value";
        case ParseStatus::HelpRequested:    return L"Help Requested";
        default:                            return L"Unknown";
    }
}

} // namespace ExplorerLens

