#include "UpdateEngine.h"
#include <sstream>

namespace ExplorerLens {
namespace Engine {

UpdateEngine::UpdateEngine() = default;

const wchar_t* UpdateEngine::GetChannelName(EngineUpdateChannel ch)
{
    switch (ch) {
        case EngineUpdateChannel::Stable:
            return L"Stable";
        case EngineUpdateChannel::Beta:
            return L"Beta";
        case EngineUpdateChannel::Nightly:
            return L"Nightly";
        case EngineUpdateChannel::Enterprise:
            return L"Enterprise";
        default:
            return L"Unknown";
    }
}

const wchar_t* UpdateEngine::GetStatusName(UpdateStatus status)
{
    switch (status) {
        case UpdateStatus::Unknown:
            return L"Unknown";
        case UpdateStatus::UpToDate:
            return L"Up to Date";
        case UpdateStatus::Available:
            return L"Available";
        case UpdateStatus::Downloading:
            return L"Downloading";
        case UpdateStatus::Verifying:
            return L"Verifying";
        case UpdateStatus::Ready:
            return L"Ready";
        case UpdateStatus::Failed:
            return L"Failed";
        default:
            return L"Unknown";
    }
}

int UpdateEngine::CompareVersions(const std::wstring& a, const std::wstring& b)
{
    auto parse = [](const std::wstring& v) -> std::vector<int> {
        std::vector<int> parts;
        std::wstringstream ss(v);
        std::wstring token;
        while (std::getline(ss, token, L'.')) {
            try {
                parts.push_back(std::stoi(token));
            } catch (...) {
                parts.push_back(0);
            }
        }
        return parts;
    };
    auto pa = parse(a), pb = parse(b);
    size_t maxLen = (pa.size() > pb.size()) ? pa.size() : pb.size();
    pa.resize(maxLen, 0);
    pb.resize(maxLen, 0);
    for (size_t i = 0; i < maxLen; ++i) {
        if (pa[i] != pb[i])
            return pa[i] - pb[i];
    }
    return 0;
}

UpdateInfo UpdateEngine::CheckForUpdate(const std::wstring& latestVersion)
{
    UpdateInfo info;
    info.version = latestVersion;
    info.channel = m_channel;
    int cmp = CompareVersions(latestVersion, m_currentVersion);
    if (cmp > 0) {
        info.status = UpdateStatus::Available;
    } else {
        info.status = UpdateStatus::UpToDate;
    }
    return info;
}

bool UpdateEngine::VerifyHash(const std::wstring& expected, const std::wstring& actual)
{
    return expected == actual;
}

}  // namespace Engine
}  // namespace ExplorerLens
