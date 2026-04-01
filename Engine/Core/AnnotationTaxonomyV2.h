// AnnotationTaxonomyV2.h — Annotation Taxonomy v2
// Copyright (c) 2026 ExplorerLens Project
//
// Hierarchical label taxonomy for annotation classification with fuzzy lookup support.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

struct ATaxV2Label {
    std::string id;
    std::string name;
    std::string parentId;
    std::string description;
};

struct ATaxV2LookupResult {
    bool                   found  = false;
    std::vector<ATaxV2Label> matches;
};

class AnnotationTaxonomyV2 {
public:
    void AddLabel(const ATaxV2Label& label) { m_labels[label.id] = label; }

    ATaxV2LookupResult Lookup(const std::string& query) const {
        ATaxV2LookupResult r;
        for (const auto& [id, label] : m_labels) {
            std::string lname = label.name;
            std::transform(lname.begin(), lname.end(), lname.begin(),
                           [](unsigned char c) { return static_cast<char>(::tolower(c)); });
            std::string q = query;
            std::transform(q.begin(), q.end(), q.begin(),
                           [](unsigned char c) { return static_cast<char>(::tolower(c)); });
            if (lname.find(q) != std::string::npos) { r.matches.push_back(label); r.found = true; }
        }
        return r;
    }
    std::vector<ATaxV2Label> Children(const std::string& parentId) const {
        std::vector<ATaxV2Label> out;
        for (const auto& [id, label] : m_labels)
            if (label.parentId == parentId) out.push_back(label);
        return out;
    }
    uint32_t LabelCount() const { return static_cast<uint32_t>(m_labels.size()); }

private:
    std::unordered_map<std::string, ATaxV2Label> m_labels;
};

}} // namespace ExplorerLens::Engine
