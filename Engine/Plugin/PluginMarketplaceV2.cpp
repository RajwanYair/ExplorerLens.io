//==============================================================================
// PluginMarketplaceV2
// Plugin marketplace implementation
//==============================================================================

#include "PluginMarketplaceV2.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// bcrypt.h requires NTSTATUS which may be missing under WIN32_LEAN_AND_MEAN
#ifndef _NTDEF_
typedef LONG NTSTATUS;
#endif
#include <bcrypt.h>

namespace ExplorerLens {
namespace Engine {

//------------------------------------------------------------------------------
PluginVersion PluginVersion::Parse(const std::wstring &str) {
 PluginVersion v;
 size_t pos1 = str.find(L'.');
 if (pos1 == std::wstring::npos) {
 try {
 v.major = static_cast<uint16_t>(std::stoi(str));
 } catch (...) {
 }
 return v;
 }
 try {
 v.major = static_cast<uint16_t>(std::stoi(str.substr(0, pos1)));
 } catch (...) {
 }
 size_t pos2 = str.find(L'.', pos1 + 1);
 if (pos2 == std::wstring::npos) {
 try {
 v.minor = static_cast<uint16_t>(std::stoi(str.substr(pos1 + 1)));
 } catch (...) {
 }
 return v;
 }
 try {
 v.minor =
 static_cast<uint16_t>(std::stoi(str.substr(pos1 + 1, pos2 - pos1 - 1)));
 } catch (...) {
 }
 size_t pos3 = str.find(L'-', pos2 + 1);
 if (pos3 != std::wstring::npos) {
 try {
 v.patch = static_cast<uint16_t>(
 std::stoi(str.substr(pos2 + 1, pos3 - pos2 - 1)));
 } catch (...) {
 }
 v.prerelease = str.substr(pos3 + 1);
 } else {
 try {
 v.patch = static_cast<uint16_t>(std::stoi(str.substr(pos2 + 1)));
 } catch (...) {
 }
 }
 return v;
}

//------------------------------------------------------------------------------
PluginMarketplaceV2::PluginMarketplaceV2()
 : m_catalogUrl(L"https://plugins.explorerlens.dev/v1") {}

PluginMarketplaceV2::PluginMarketplaceV2(const std::wstring &catalogUrl)
 : m_catalogUrl(catalogUrl) {}

//------------------------------------------------------------------------------
void PluginMarketplaceV2::AddToCatalog(const PluginListing &plugin) {
 m_catalog.push_back(plugin);
}

//------------------------------------------------------------------------------
MarketplaceResult
PluginMarketplaceV2::Search(const MarketplaceFilter &filter) const {
 MarketplaceResult result;
 for (const auto &p : m_catalog) {
 bool matches = true;
 if (!filter.query.empty()) {
 bool nameMatch = p.name.find(filter.query) != std::wstring::npos;
 bool descMatch = p.description.find(filter.query) != std::wstring::npos;
 if (!nameMatch && !descMatch)
 matches = false;
 }
 if (filter.verifiedOnly && !p.isVerified)
 matches = false;
 if (filter.compatibleOnly && !p.isCompatible)
 matches = false;
 if (matches)
 result.plugins.push_back(p);
 }
 result.totalCount = static_cast<uint32_t>(result.plugins.size());

 // Sort
 switch (filter.sortBy) {
 case MarketplaceFilter::Downloads:
 std::sort(
 result.plugins.begin(), result.plugins.end(),
 [](const auto &a, const auto &b) { return a.downloads > b.downloads; });
 break;
 case MarketplaceFilter::Rating:
 std::sort(result.plugins.begin(), result.plugins.end(),
 [](const auto &a, const auto &b) { return a.rating > b.rating; });
 break;
 default:
 break;
 }

 // Limit results
 if (result.plugins.size() > filter.maxResults) {
 result.hasMore = true;
 result.plugins.resize(filter.maxResults);
 }
 return result;
}

//------------------------------------------------------------------------------
bool PluginMarketplaceV2::Install(const PluginListing &plugin) {
 InstalledPlugin ip;
 ip.listing = plugin;
 ip.state = PluginInstallState::Installed;
 ip.installedVersion = plugin.version;
 ip.installPath = L"C:\\ProgramData\\ExplorerLens\\plugins\\" + plugin.id;
 m_installed.push_back(ip);
 return true;
}

//------------------------------------------------------------------------------
bool PluginMarketplaceV2::Uninstall(const std::wstring &pluginId) {
 auto it = std::remove_if(
 m_installed.begin(), m_installed.end(),
 [&](const InstalledPlugin &ip) { return ip.listing.id == pluginId; });
 if (it == m_installed.end())
 return false;
 m_installed.erase(it, m_installed.end());
 return true;
}

//------------------------------------------------------------------------------
std::vector<PluginListing> PluginMarketplaceV2::CheckUpdates() const {
 std::vector<PluginListing> updates;
 for (const auto &installed : m_installed) {
 for (const auto &catalog : m_catalog) {
 if (catalog.id == installed.listing.id &&
 catalog.version >= installed.installedVersion &&
 !(catalog.version == installed.installedVersion)) {
 updates.push_back(catalog);
 }
 }
 }
 return updates;
}

//------------------------------------------------------------------------------
PluginInstallState
PluginMarketplaceV2::GetState(const std::wstring &pluginId) const {
 for (const auto &ip : m_installed) {
 if (ip.listing.id == pluginId)
 return ip.state;
 }
 return PluginInstallState::NotInstalled;
}

//------------------------------------------------------------------------------
bool PluginMarketplaceV2::VerifyHash(const std::wstring &filePath,
 const std::wstring &expectedHash) {
 if (expectedHash.empty())
 return false;

 // Open file and read contents
 std::ifstream file(filePath, std::ios::binary);
 if (!file.is_open())
 return false;

 std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(file)),
 std::istreambuf_iterator<char>());
 file.close();

 // Compute SHA-256 via BCrypt
 BCRYPT_ALG_HANDLE hAlg = nullptr;
 BCRYPT_HASH_HANDLE hHash = nullptr;
 NTSTATUS status =
 BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
 if (status != 0 || !hAlg)
 return false;

 status = BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
 if (status != 0 || !hHash) {
 BCryptCloseAlgorithmProvider(hAlg, 0);
 return false;
 }

 status = BCryptHashData(hHash, fileData.data(),
 static_cast<ULONG>(fileData.size()), 0);
 if (status != 0) {
 BCryptDestroyHash(hHash);
 BCryptCloseAlgorithmProvider(hAlg, 0);
 return false;
 }

 uint8_t hashBytes[32]{}; // SHA-256 = 32 bytes
 status = BCryptFinishHash(hHash, hashBytes, 32, 0);
 BCryptDestroyHash(hHash);
 BCryptCloseAlgorithmProvider(hAlg, 0);
 if (status != 0)
 return false;

 // Convert to hex string (lowercase)
 std::wostringstream hexStream;
 hexStream << std::hex << std::setfill(L'0');
 for (int i = 0; i < 32; ++i)
 hexStream << std::setw(2) << static_cast<unsigned>(hashBytes[i]);

 std::wstring computed = hexStream.str();

 // Case-insensitive comparison
 std::wstring expected = expectedHash;
 std::transform(expected.begin(), expected.end(), expected.begin(),
 ::towlower);
 return computed == expected;
}

//------------------------------------------------------------------------------
const wchar_t *PluginMarketplaceV2::GetCategoryName(PluginCategory cat) {
 switch (cat) {
 case PluginCategory::Decoder:
 return L"Decoder";
 case PluginCategory::Renderer:
 return L"Renderer";
 case PluginCategory::PostProcessor:
 return L"Post-Processor";
 case PluginCategory::CacheProvider:
 return L"Cache Provider";
 case PluginCategory::Integration:
 return L"Integration";
 case PluginCategory::Utility:
 return L"Utility";
 default:
 return L"Unknown";
 }
}

const wchar_t *PluginMarketplaceV2::GetStateName(PluginInstallState state) {
 switch (state) {
 case PluginInstallState::NotInstalled:
 return L"Not Installed";
 case PluginInstallState::Downloading:
 return L"Downloading";
 case PluginInstallState::Verifying:
 return L"Verifying";
 case PluginInstallState::Installing:
 return L"Installing";
 case PluginInstallState::Installed:
 return L"Installed";
 case PluginInstallState::UpdateAvailable:
 return L"Update Available";
 case PluginInstallState::Failed:
 return L"Failed";
 case PluginInstallState::Disabled:
 return L"Disabled";
 default:
 return L"Unknown";
 }
}

} // namespace Engine
} // namespace ExplorerLens
