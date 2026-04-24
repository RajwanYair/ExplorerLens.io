//==============================================================================
// ExplorerLens Engine — COM boundary guards (Sprint S233)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §2.1 ADR-021: no STL containers across the COM DLL boundary
//==============================================================================
//
// Windows COM is an ABI: types that cross a COM boundary must be trivially
// copyable, POD-layout, or COM-registered interface pointers. Passing
// std::string / std::vector / std::expected across the boundary is a crash
// waiting to happen the moment a caller built against a different MSVC STL
// dispatches through the interface.
//
// This header provides `static_assert` helpers that any type declared to
// cross a COM boundary can opt into. The goal: catch violations at compile
// time rather than in production.
//==============================================================================
#pragma once

#include <type_traits>

namespace ExplorerLens {
namespace Engine {
namespace COM {

/// <summary>
/// Tag type used by CheckCOMBoundary to mark a structure as crossing
/// the COM boundary. Instantiate with your POD type; compilation fails
/// if the type is not suitable.
/// </summary>
template <typename T>
struct CheckCOMBoundary
{
    static_assert(std::is_trivially_copyable_v<T>,
        "Type crossing COM boundary must be trivially copyable.");
    static_assert(std::is_standard_layout_v<T>,
        "Type crossing COM boundary must have standard layout.");
    static_assert(!std::is_polymorphic_v<T>,
        "Type crossing COM boundary must not have virtual methods.");
    static constexpr bool value = true;
};

/// <summary>
/// Macro form: `LENS_COM_BOUNDARY_POD(MyStruct);` at namespace scope.
/// </summary>
#define LENS_COM_BOUNDARY_POD(T) \
    static_assert(::ExplorerLens::Engine::COM::CheckCOMBoundary<T>::value, \
        "COM boundary violation: " #T)

/// <summary>
/// Example POD permitted across COM boundary: decode request descriptor.
/// </summary>
struct COMDecodeRequest
{
    unsigned long long pathHash;
    unsigned long long fileSize;
    unsigned int       thumbSize;
    unsigned int       flags;
};

LENS_COM_BOUNDARY_POD(COMDecodeRequest);

/// <summary>
/// Runtime helper: confirms an argument pointer is non-null and aligned
/// for the target type. Use at the very top of every COM interface method.
/// </summary>
template <typename T>
[[nodiscard]] constexpr bool IsValidCOMArg(const T* ptr) noexcept
{
    return ptr != nullptr
        && (reinterpret_cast<unsigned long long>(ptr) % alignof(T)) == 0;
}

} // namespace COM
} // namespace Engine
} // namespace ExplorerLens
