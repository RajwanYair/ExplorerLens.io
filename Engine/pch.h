// Engine/pch.h — Precompiled Header for ExplorerLensEngine
// Copyright (c) 2026 ExplorerLens Project
//
// Includes stable, rarely-changed system and C++ standard library headers.
// All Engine .cpp files should benefit from this PCH without needing to
// include these headers individually.
//
// DO NOT include project-local headers here — they change frequently and
// would invalidate the PCH on every edit.
//
#pragma once

// ---------------------------------------------------------------------------
// C++ Standard Library — commonly used across Engine
// ---------------------------------------------------------------------------

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------
// Windows SDK (only when building on Windows)
// ---------------------------------------------------------------------------

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <winerror.h>
#  include <unknwn.h>
#endif
