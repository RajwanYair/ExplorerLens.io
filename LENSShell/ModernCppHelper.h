#pragma once

// Modern C++17 utilities for ExplorerLens
// Provides filesystem operations and string conversions

#include <filesystem>
#include <string>
#include <string_view>
#include <memory>
#include <optional>

namespace fs = std::filesystem;

namespace ModernCpp
{
	// Convert between wide and narrow strings
	inline std::wstring ToWideString(std::string_view str)
	{
		if (str.empty()) return {};
		
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
		std::wstring result(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), result.data(), size_needed);
		return result;
	}

	inline std::string ToNarrowString(std::wstring_view wstr)
	{
		if (wstr.empty()) return {};
		
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
		std::string result(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), result.data(), size_needed, nullptr, nullptr);
		return result;
	}

	// Modern filesystem utilities
	inline std::optional<fs::path> GetFileExtension(const fs::path& filePath)
	{
		if (!filePath.has_extension())
			return std::nullopt;
		return filePath.extension();
	}

	inline std::wstring GetFileExtensionLower(const fs::path& filePath)
	{
		auto ext = filePath.extension().wstring();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
		return ext;
	}

	inline bool FileExists(const fs::path& filePath) noexcept
	{
		std::error_code ec;
		return fs::exists(filePath, ec) && fs::is_regular_file(filePath, ec);
	}

	inline std::optional<uintmax_t> GetFileSize(const fs::path& filePath) noexcept
	{
		std::error_code ec;
		auto size = fs::file_size(filePath, ec);
		if (ec)
			return std::nullopt;
		return size;
	}

	inline std::optional<fs::file_time_type> GetFileModificationTime(const fs::path& filePath) noexcept
	{
		std::error_code ec;
		auto time = fs::last_write_time(filePath, ec);
		if (ec)
			return std::nullopt;
		return time;
	}

	// Convert fs::file_time_type to FILETIME
	inline FILETIME FileTimeToWin32(const fs::file_time_type& ft)
	{
		// Convert file_time to system_clock time
		auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
			ft - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
		);
		
		// Convert to time_t
		auto tt = std::chrono::system_clock::to_time_t(sctp);
		
		// Convert to FILETIME (100-nanosecond intervals since 1601-01-01)
		ULARGE_INTEGER ull;
		ull.QuadPart = (tt * 10000000LL) + 116444736000000000LL;
		
		FILETIME result;
		result.dwLowDateTime = ull.LowPart;
		result.dwHighDateTime = ull.HighPart;
		return result;
	}

	// Smart pointer type aliases for common Windows handles
	struct HandleDeleter
	{
		void operator()(HANDLE h) const
		{
			if (h && h != INVALID_HANDLE_VALUE)
				CloseHandle(h);
		}
	};

	using UniqueHandle = std::unique_ptr<void, HandleDeleter>;

	inline UniqueHandle MakeUniqueHandle(HANDLE h)
	{
		return UniqueHandle(h);
	}

	// RAII wrapper for GDI objects
	template<typename T>
	class GdiObjectPtr
	{
	public:
		explicit GdiObjectPtr(T obj = nullptr) : m_obj(obj) {}
		~GdiObjectPtr() { if (m_obj) DeleteObject(m_obj); }
		
		GdiObjectPtr(const GdiObjectPtr&) = delete;
		GdiObjectPtr& operator=(const GdiObjectPtr&) = delete;
		
		GdiObjectPtr(GdiObjectPtr&& other) noexcept : m_obj(other.m_obj)
		{
			other.m_obj = nullptr;
		}
		
		GdiObjectPtr& operator=(GdiObjectPtr&& other) noexcept
		{
			if (this != &other)
			{
				if (m_obj) DeleteObject(m_obj);
				m_obj = other.m_obj;
				other.m_obj = nullptr;
			}
			return *this;
		}
		
		T get() const { return m_obj; }
		T* operator&() { return &m_obj; }
		operator T() const { return m_obj; }
		
		T release()
		{
			T obj = m_obj;
			m_obj = nullptr;
			return obj;
		}
		
	private:
		T m_obj;
	};

	using BitmapPtr = GdiObjectPtr<HBITMAP>;
	using BrushPtr = GdiObjectPtr<HBRUSH>;
	using PenPtr = GdiObjectPtr<HPEN>;
}

