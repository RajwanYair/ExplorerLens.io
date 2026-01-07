// Error Logging System for DarkThumbs
// Provides structured error logging with categories and severity levels

#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <windows.h>

// Undefine Windows macros that conflict with our enum values
#ifdef DEBUG
#undef DEBUG
#endif
#ifdef ERROR
#undef ERROR
#endif

namespace DarkThumbs {

enum class LogLevel {
  LVL_DEBUG = 0,
  LVL_INFO = 1,
  LVL_WARNING = 2,
  LVL_ERROR = 3,
  LVL_CRITICAL = 4
};

enum class LogCategory { GENERAL, GPU, CACHE, DECODER, COM, PERFORMANCE };

class Logger {
private:
  std::wstring m_logPath;
  std::ofstream m_logFile;
  std::mutex m_mutex;
  LogLevel m_minLevel;
  bool m_enabled;
  bool m_debugMode;

  Logger()
      : m_minLevel(LogLevel::LVL_WARNING), m_enabled(false),
        m_debugMode(false) {
    // Check if debug mode is enabled via registry
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\DarkThumbs\\Settings", 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {
      DWORD debugMode = 0;
      DWORD size = sizeof(DWORD);
      if (RegQueryValueExW(hKey, L"DebugLogging", nullptr, nullptr,
                           (LPBYTE)&debugMode, &size) == ERROR_SUCCESS) {
        m_debugMode = (debugMode != 0);
      }
      RegCloseKey(hKey);
    }

    if (m_debugMode) {
      InitializeLogFile();
    }
  }

  void InitializeLogFile() {
    wchar_t localAppData[MAX_PATH];
    if (SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0,
                         localAppData) == S_OK) {
      m_logPath = localAppData;
      m_logPath += L"\\DarkThumbs\\logs";

      // Create directory if it doesn't exist
      SHCreateDirectoryExW(nullptr, m_logPath.c_str(), nullptr);

      // Generate log filename with date
      auto now = std::chrono::system_clock::now();
      auto time_t_now = std::chrono::system_clock::to_time_t(now);
      std::tm tm_now;
      localtime_s(&tm_now, &time_t_now);

      std::wostringstream filename;
      filename << m_logPath << L"\\DarkThumbs_"
               << std::put_time(&tm_now, L"%Y%m%d_%H%M%S") << L".log";

      m_logPath = filename.str();
      m_logFile.open(m_logPath, std::ios::out | std::ios::app);

      if (m_logFile.is_open()) {
        m_enabled = true;
        m_minLevel = LogLevel::LVL_DEBUG;

        // Write header
        WriteHeader();
      }
    }
  }

  void WriteHeader() {
    m_logFile << "========================================\n";
    m_logFile << "DarkThumbs Error Log\n";
    m_logFile << "Version: 5.2.0\n";
    m_logFile << "Date: " << GetTimestamp() << "\n";
    m_logFile << "========================================\n\n";
    m_logFile.flush();
  }

  std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) %
              1000;

    std::tm tm_now;
    localtime_s(&tm_now, &time_t_now);

    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << '.'
        << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
  }

  std::string LevelToString(LogLevel level) {
    switch (level) {
    case LogLevel::LVL_DEBUG:
      return "DEBUG";
    case LogLevel::LVL_INFO:
      return "INFO";
    case LogLevel::LVL_WARNING:
      return "WARN";
    case LogLevel::LVL_ERROR:
      return "ERROR";
    case LogLevel::LVL_CRITICAL:
      return "CRIT";
    default:
      return "UNKNOWN";
    }
  }

  std::string CategoryToString(LogCategory category) {
    switch (category) {
    case LogCategory::GPU:
      return "GPU";
    case LogCategory::CACHE:
      return "CACHE";
    case LogCategory::DECODER:
      return "DECODER";
    case LogCategory::COM:
      return "COM";
    case LogCategory::PERFORMANCE:
      return "PERF";
    case LogCategory::GENERAL:
    default:
      return "GENERAL";
    }
  }

public:
  static Logger &Instance() {
    static Logger instance;
    return instance;
  }

  ~Logger() {
    if (m_logFile.is_open()) {
      m_logFile << "\n========================================\n";
      m_logFile << "Log session ended: " << GetTimestamp() << "\n";
      m_logFile << "========================================\n";
      m_logFile.close();
    }
  }

  void Log(LogLevel level, LogCategory category, const std::string &message,
           const char *file = nullptr, int line = 0) {
    if (!m_enabled || level < m_minLevel) {
      return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_logFile.is_open()) {
      m_logFile << "[" << GetTimestamp() << "] "
                << "[" << LevelToString(level) << "] "
                << "[" << CategoryToString(category) << "] ";

      if (file && line > 0) {
        // Extract filename from full path
        std::string filename = file;
        size_t pos = filename.find_last_of("\\/");
        if (pos != std::string::npos) {
          filename = filename.substr(pos + 1);
        }
        m_logFile << filename << ":" << line << " - ";
      }

      m_logFile << message << "\n";
      m_logFile.flush();
    }

    // Also output to debug output for development
    if (IsDebuggerPresent()) {
      std::string debugMsg = "[" + LevelToString(level) + "] " + message + "\n";
      OutputDebugStringA(debugMsg.c_str());
    }
  }

  void LogHRESULT(LogLevel level, LogCategory category,
                  const std::string &operation, HRESULT hr,
                  const char *file = nullptr, int line = 0) {
    std::ostringstream msg;
    msg << operation << " failed with HRESULT 0x" << std::hex << std::uppercase
        << hr;

    // Try to get error description
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer, 0, nullptr);

    if (size > 0 && messageBuffer) {
      msg << " (" << messageBuffer << ")";
      LocalFree(messageBuffer);
    }

    Log(level, category, msg.str(), file, line);
  }

  void SetMinLevel(LogLevel level) { m_minLevel = level; }

  bool IsEnabled() const { return m_enabled; }

  std::wstring GetLogPath() const { return m_logPath; }
};

// Convenience macros with DT_ prefix to avoid conflicts
#define DT_LOG_DEBUG(category, msg)                                            \
  DarkThumbs::Logger::Instance().Log(DarkThumbs::LogLevel::LVL_DEBUG,          \
                                     category, msg, __FILE__, __LINE__)
#define DT_LOG_INFO(category, msg)                                             \
  DarkThumbs::Logger::Instance().Log(DarkThumbs::LogLevel::LVL_INFO, category, \
                                     msg, __FILE__, __LINE__)
#define DT_LOG_WARNING(category, msg)                                          \
  DarkThumbs::Logger::Instance().Log(DarkThumbs::LogLevel::LVL_WARNING,        \
                                     category, msg, __FILE__, __LINE__)
#define DT_LOG_ERROR(category, msg)                                            \
  DarkThumbs::Logger::Instance().Log(DarkThumbs::LogLevel::LVL_ERROR,          \
                                     category, msg, __FILE__, __LINE__)
#define DT_LOG_CRITICAL(category, msg)                                         \
  DarkThumbs::Logger::Instance().Log(DarkThumbs::LogLevel::LVL_CRITICAL,       \
                                     category, msg, __FILE__, __LINE__)
#define DT_LOG_HRESULT(level, category, operation, hr)                         \
  DarkThumbs::Logger::Instance().LogHRESULT(level, category, operation, hr,    \
                                            __FILE__, __LINE__)

} // namespace DarkThumbs
