//==============================================================================
// ExplorerLens Engine - Decoder Registry Implementation
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "DecoderRegistry.h"
#include <algorithm>
#include <cwchar>

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// Constructor / Destructor
//==============================================================================

DecoderRegistry::DecoderRegistry()
{
}

DecoderRegistry::~DecoderRegistry()
{
 Clear();
}

//==============================================================================
// Public Methods
//==============================================================================

bool DecoderRegistry::RegisterDecoder(IThumbnailDecoder* decoder)
{
 if (!decoder) {
 return false;
 }
 
 // Check if already registered
 auto it = std::find(m_decoders.begin(), m_decoders.end(), decoder);
 if (it != m_decoders.end()) {
 return false; // Already registered
 }
 
 m_decoders.push_back(decoder);
 return true;
}

bool DecoderRegistry::UnregisterDecoder(IThumbnailDecoder* decoder)
{
 if (!decoder) {
 return false;
 }
 
 auto it = std::find(m_decoders.begin(), m_decoders.end(), decoder);
 if (it != m_decoders.end()) {
 // NOTE: DecoderRegistry is non-owning - it does NOT delete decoder pointers
 // The caller is responsible for decoder lifetime management
 m_decoders.erase(it);
 return true;
 }
 
 return false;
}

IThumbnailDecoder* DecoderRegistry::FindDecoder(const wchar_t* filePath) const
{
 if (!filePath) {
 return nullptr;
 }
 
 // Check each decoder to see if it can handle this file
 for (IThumbnailDecoder* decoder : m_decoders) {
 if (decoder && decoder->CanDecode(filePath)) {
 return decoder;
 }
 }
 
 return nullptr;
}

IThumbnailDecoder* DecoderRegistry::FindDecoderByName(const wchar_t* name) const
{
 if (!name) {
 return nullptr;
 }
 
 for (IThumbnailDecoder* decoder : m_decoders) {
 if (decoder) {
 const wchar_t* decoderName = decoder->GetName();
 if (decoderName && wcscmp(decoderName, name) == 0) {
 return decoder;
 }
 }
 }
 
 return nullptr;
}

size_t DecoderRegistry::GetDecoderCount() const
{
 return m_decoders.size();
}

IThumbnailDecoder* DecoderRegistry::GetDecoder(size_t index) const
{
 if (index >= m_decoders.size()) {
 return nullptr;
 }
 
 return m_decoders[index];
}

const std::vector<IThumbnailDecoder*>& DecoderRegistry::GetAllDecoders() const
{
 return m_decoders;
}

void DecoderRegistry::Clear()
{
 // NOTE: DecoderRegistry is non-owning - it does NOT delete decoder pointers
 // The caller is responsible for decoder lifetime management
 // Simply clear the vector of pointers
 m_decoders.clear();
}

void DecoderRegistry::GetStats(
 size_t* outTotalDecoders,
 size_t* outImageDecoders,
 size_t* outArchiveDecoders,
 size_t* outTotalExtensions) const
{
 size_t totalDecoders = m_decoders.size();
 size_t imageDecoders = 0;
 size_t archiveDecoders = 0;
 size_t totalExtensions = 0;
 
 for (const IThumbnailDecoder* decoder : m_decoders) {
 if (!decoder) {
 continue;
 }
 
 // Count decoder types
 if (decoder->IsArchiveDecoder()) {
 archiveDecoders++;
 } else {
 imageDecoders++;
 }
 
 // Count extensions
 totalExtensions += decoder->GetExtensionCount();
 }
 
 // Set output parameters
 if (outTotalDecoders) {
 *outTotalDecoders = totalDecoders;
 }
 
 if (outImageDecoders) {
 *outImageDecoders = imageDecoders;
 }
 
 if (outArchiveDecoders) {
 *outArchiveDecoders = archiveDecoders;
 }
 
 if (outTotalExtensions) {
 *outTotalExtensions = totalExtensions;
 }
}

} // namespace Engine
} // namespace ExplorerLens

