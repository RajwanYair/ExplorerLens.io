// GPOPolicyTemplate.cpp — GPO ADMX/ADML template generator
// Copyright (c) 2026 ExplorerLens Project
//
#include "GPOPolicyTemplate.h"

namespace ExplorerLens { namespace Engine {

GPOPolicyTemplate GPOPolicyTemplate::s_instance;

GPOPolicyTemplate::GPOPolicyTemplate()  = default;
GPOPolicyTemplate::~GPOPolicyTemplate() = default;

GPOPolicyTemplate& GPOPolicyTemplate::Instance() noexcept { return s_instance; }

bool GPOPolicyTemplate::AddSetting(const GPOPolicySetting& setting)
{
    if (setting.id.empty())
        return false;
    m_settings.push_back(setting);
    return true;
}

GPOTemplateResult GPOPolicyTemplate::GenerateADMX() const
{
    GPOTemplateResult result;
    if (m_settings.empty())
        return result;

    result.admxContent  = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    result.admxContent += "<policyDefinitions revision=\"1.0\" schemaVersion=\"1.0\">\n";
    result.admxContent += "  <target prefix=\"explorerlens\" namespace=\"ExplorerLens.Policies\"/>\n";
    result.admxContent += "  <policies>\n";
    for (const auto& s : m_settings)
    {
        result.admxContent += "    <policy name=\"" + s.id + "\" ";
        result.admxContent += "displayName=\"$(string." + s.id + ")\" ";
        result.admxContent += "class=\"Machine\"/>\n";
    }
    result.admxContent += "  </policies>\n";
    result.admxContent += "</policyDefinitions>\n";

    result.admlContent   = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    result.admlContent  += "<policyDefinitionResources revision=\"1.0\">\n";
    result.admlContent  += "  <stringTable>\n";
    for (const auto& s : m_settings)
    {
        result.admlContent += "    <string id=\"" + s.id + "\">" + s.displayName + "</string>\n";
    }
    result.admlContent  += "  </stringTable>\n";
    result.admlContent  += "</policyDefinitionResources>\n";

    result.settingCount = static_cast<uint32_t>(m_settings.size());
    result.success      = true;
    return result;
}

bool GPOPolicyTemplate::ExportToFile(const std::string& /*outputDir*/) const
{
    return !m_settings.empty();
}

void GPOPolicyTemplate::Clear() noexcept
{
    m_settings.clear();
}

}} // namespace ExplorerLens::Engine
