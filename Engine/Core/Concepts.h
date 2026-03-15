// Concepts.h — C++20 Concepts for ExplorerLens Type Constraints
// Copyright (c) 2026 ExplorerLens Project
//
// Compile-time concepts that replace runtime interface checks with
// zero-cost type constraints. Inspired by RegiLattice's declarative
// pattern — decoders are validated at compile time, not runtime.
//
#pragma once

#include <concepts>
#include <type_traits>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

struct HBITMAP__;
typedef HBITMAP__* HBITMAP;

namespace ExplorerLens {
namespace Engine {

// Forward declarations
struct ThumbnailRequest;
struct ThumbnailResult;
struct DecoderInfo;

//==============================================================================
// Core Decoder Concept
//
// Any type satisfying ThumbnailDecoderConcept can participate in the
// decode pipeline without virtual dispatch overhead.
//==============================================================================
template <typename T>
concept ThumbnailDecoderConcept = requires(T decoder,
                                           const wchar_t* path,
                                           const ThumbnailRequest& req,
                                           ThumbnailResult& res) {
    { decoder.CanDecode(path) } -> std::convertible_to<bool>;
    { decoder.Decode(req, res) } -> std::convertible_to<long>;
    { decoder.GetName() } -> std::convertible_to<const wchar_t*>;
    { decoder.GetExtensionCount() } -> std::convertible_to<uint32_t>;
    { decoder.SupportsGPU() } -> std::convertible_to<bool>;
    { decoder.IsArchiveDecoder() } -> std::convertible_to<bool>;
};

//==============================================================================
// Format Detector Concept
//==============================================================================
template <typename T>
concept FormatDetectorConcept = requires(T detector,
                                         const wchar_t* path,
                                         const uint8_t* header,
                                         size_t headerSize) {
    { detector.DetectFromExtension(path) } -> std::convertible_to<uint32_t>;
    { detector.DetectFromMagicBytes(header, headerSize) } -> std::convertible_to<uint32_t>;
};

//==============================================================================
// Cache Provider Concept
//==============================================================================
template <typename T>
concept CacheProviderConcept = requires(T cache,
                                         const wchar_t* key,
                                         uint32_t width,
                                         uint32_t height) {
    { cache.Lookup(key, width, height) } -> std::convertible_to<HBITMAP>;
    { cache.Store(key, HBITMAP{}, width, height) } -> std::convertible_to<bool>;
    { cache.Invalidate(key) } -> std::convertible_to<void>;
    { cache.Clear() } -> std::convertible_to<void>;
};

//==============================================================================
// GPU Renderer Concept
//==============================================================================
template <typename T>
concept GPURendererConcept = requires(T renderer) {
    { renderer.IsAvailable() } -> std::convertible_to<bool>;
    { renderer.GetDeviceName() } -> std::convertible_to<const wchar_t*>;
};

//==============================================================================
// Serializable Concept (for config/snapshot export, like RegiLattice's JSON)
//==============================================================================
template <typename T>
concept Serializable = requires(T obj) {
    { obj.ToJson() } -> std::convertible_to<std::string>;
};

//==============================================================================
// Pipeline Stage Concept
//==============================================================================
template <typename T>
concept PipelineStageConcept = requires(T stage) {
    { stage.GetName() } -> std::convertible_to<const char*>;
    { stage.IsEnabled() } -> std::convertible_to<bool>;
    { stage.GetPriority() } -> std::convertible_to<int>;
};

//==============================================================================
// Plugin Concept
//==============================================================================
template <typename T>
concept PluginConcept = requires(T plugin) {
    { plugin.GetPluginName() } -> std::convertible_to<const char*>;
    { plugin.GetPluginVersion() } -> std::convertible_to<const char*>;
    { plugin.Initialize() } -> std::convertible_to<bool>;
    { plugin.Shutdown() } -> std::convertible_to<void>;
};

//==============================================================================
// Compile-time helpers for concept-constrained dispatch
//==============================================================================

/// Check if a type satisfies multiple concepts simultaneously
template <typename T>
concept FullDecoder = ThumbnailDecoderConcept<T> && requires(T d) {
    { d.GetInfo() } -> std::convertible_to<DecoderInfo>;
};

/// Constraint for types that can be used as allocators in the memory subsystem
template <typename T>
concept PoolAllocator = requires(T alloc, size_t size) {
    { alloc.Allocate(size) } -> std::convertible_to<void*>;
    { alloc.Deallocate(std::declval<void*>(), size) } -> std::convertible_to<void>;
    { alloc.GetPoolSize() } -> std::convertible_to<size_t>;
};

} // namespace Engine
} // namespace ExplorerLens
