// ImageDescriptionSynthesizer.cpp — AI Image Description Synthesizer
// Copyright (c) 2026 ExplorerLens Project
//
#include "ImageDescriptionSynthesizer.h"

namespace ExplorerLens::Engine {

SynthesisResult ImageDescriptionSynthesizer::Synthesize(const void*, uint32_t, uint32_t, DescriptionDepth)
{
    return {};
}
void ImageDescriptionSynthesizer::SetLanguage(DescriptionLanguage language)
{
    m_language = language;
}
bool ImageDescriptionSynthesizer::LoadModel(const std::string&)
{
    return false;
}
DescriptionLanguage ImageDescriptionSynthesizer::GetLanguage() const
{
    return m_language;
}
bool ImageDescriptionSynthesizer::IsModelLoaded() const
{
    return false;
}

}  // namespace ExplorerLens::Engine
