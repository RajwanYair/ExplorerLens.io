// FormatSchemaValidator.h — Format Schema Validator
// Copyright (c) 2026 ExplorerLens Project
//
// Validates binary file payloads against formal format specifications,
// reporting violations with byte-level offsets and severity grades.
//
#pragma once
#include <cstdint>
#include <string>
#include "../Utils/MalformedInputHandler.h"

namespace ExplorerLens {
namespace Engine {

struct SchemaSpec
{
    std::string formatName;
    uint32_t ruleCount = 0;
    std::string specVersion;
};

struct ValidationResult
{
    bool isValid = false;
    ValidationSeverity severity = ValidationSeverity::Info;
    uint64_t byteOffset = 0;
    std::string ruleName;
};

class FormatSchemaValidator
{
  public:
    FormatSchemaValidator() = default;
};

}  // namespace Engine
}  // namespace ExplorerLens
