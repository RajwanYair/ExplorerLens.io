// VCSBadgeAdapter.h — Unified VCS Badge Rendering Adapter
// Copyright (c) 2026 ExplorerLens Project
//
// Abstracts over multiple VCS backends (Git, SVN, Mercurial, Perforce) to
// produce a normalised badge descriptor used by the overlay compositor.
//
#pragma once
#include <string>
#include <cstdint>
#include <filesystem>

namespace ExplorerLens {
namespace Engine {

enum class VCSProvider { Git, SVN, Mercurial, Perforce, Unknown };
enum class VCSBadgeType { Status, BranchName, CommitHash, TagName, None };

struct VCSBadgeDescriptor {
    VCSProvider  provider    = VCSProvider::Unknown;
    VCSBadgeType badgeType   = VCSBadgeType::None;
    std::string  label;
    uint32_t     bgColor     = 0xFF3C3C3CU;
    uint32_t     fgColor     = 0xFFFFFFFFU;
    bool         valid       = false;
};

struct VCSBadgeConfig {
    VCSProvider  preferredProvider = VCSProvider::Git;
    VCSBadgeType badgeType         = VCSBadgeType::Status;
    bool         autoDetect        = true;
};

class VCSBadgeAdapter {
public:
    explicit VCSBadgeAdapter(const VCSBadgeConfig& cfg = {}) : m_cfg(cfg) {}

    VCSBadgeDescriptor  Build(const std::string& path) const {
        VCSBadgeDescriptor d;
        if (path.empty()) return d;
        d.provider  = m_cfg.autoDetect ? DetectProvider(path) : m_cfg.preferredProvider;
        d.badgeType = m_cfg.badgeType;
        d.label     = "M";
        d.valid     = true;
        return d;
    }

    // Walks up the directory tree looking for VCS sentinel dirs/files.
    // Returns first match: .git → Git, .svn → SVN, .hg → Mercurial, .p4config → Perforce.
    VCSProvider  DetectProvider(const std::string& path) const {
        if (path.empty()) return VCSProvider::Unknown;
        namespace fs = std::filesystem;
        fs::path dir = fs::path(path).is_absolute()
            ? (fs::is_directory(path) ? fs::path(path) : fs::path(path).parent_path())
            : fs::absolute(path).parent_path();
        while (!dir.empty()) {
            if (fs::exists(dir / ".git"))     return VCSProvider::Git;
            if (fs::exists(dir / ".svn"))     return VCSProvider::SVN;
            if (fs::exists(dir / ".hg"))      return VCSProvider::Mercurial;
            if (fs::exists(dir / ".p4config"))return VCSProvider::Perforce;
            fs::path parent = dir.parent_path();
            if (parent == dir) break;
            dir = parent;
        }
        return m_cfg.preferredProvider;
    }

    void  SetConfig(const VCSBadgeConfig& cfg) { m_cfg = cfg; }
    const VCSBadgeConfig& GetConfig()    const { return m_cfg; }

    static const wchar_t* ProviderName(VCSProvider p) {
        switch (p) {
            case VCSProvider::Git:       return L"Git";
            case VCSProvider::SVN:       return L"SVN";
            case VCSProvider::Mercurial: return L"Hg";
            case VCSProvider::Perforce:  return L"P4";
            default:                     return L"Unknown";
        }
    }

private:
    VCSBadgeConfig m_cfg;
};

} // namespace Engine
} // namespace ExplorerLens
