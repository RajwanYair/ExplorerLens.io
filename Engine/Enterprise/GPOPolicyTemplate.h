// GPOPolicyTemplate.h — Windows Group Policy Object template generator
// Copyright (c) 2026 ExplorerLens Project
//
// Generates ADMX/ADML Group Policy Object templates for ExplorerLens enterprise
// deployment. Templates define thumbnail quality, cache limits, GPU policy, and
// plugin restrictions that GPO administrators can push via Active Directory.
//
#pragma once

#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct GPOPolicySetting
{
    std::string id;
    std::string displayName;
    std::string description;
    std::string defaultValue;
    std::string category;
};

struct GPOTemplateResult
{
    bool        success      = false;
    std::string admxContent;
    std::string admlContent;
    uint32_t    settingCount = 0;
};

class GPOPolicyTemplate
{
public:
    GPOPolicyTemplate();
    ~GPOPolicyTemplate();

    GPOPolicyTemplate(const GPOPolicyTemplate&)            = delete;
    GPOPolicyTemplate& operator=(const GPOPolicyTemplate&) = delete;

    bool               AddSetting(const GPOPolicySetting& setting);
    GPOTemplateResult  GenerateADMX() const;
    bool               ExportToFile(const std::string& outputDir) const;
    uint32_t           SettingCount() const noexcept { return static_cast<uint32_t>(m_settings.size()); }
    void               Clear() noexcept;

    static GPOPolicyTemplate& Instance() noexcept;

private:
    std::vector<GPOPolicySetting> m_settings;
    static GPOPolicyTemplate      s_instance;
};

}} // namespace ExplorerLens::Engine
