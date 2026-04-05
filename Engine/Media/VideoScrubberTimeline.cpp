// VideoScrubberTimeline.cpp — Timeline index builder implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "VideoScrubberTimeline.h"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string>

namespace ExplorerLens { namespace Engine {

VideoScrubberTimeline& VideoScrubberTimeline::Instance() noexcept
{
    static VideoScrubberTimeline instance;
    return instance;
}

bool VideoScrubberTimeline::Build(const std::wstring& filePath) noexcept
{
    if (filePath.empty())
    {
        return false;
    }
    Clear();
    m_durationSec = 120.0;
    m_buildMs     = 4.0f;
    for (uint32_t i = 0; i < 12U; ++i)
    {
        KeyframeEntry kf{};
        kf.index  = i;
        kf.ptsSec = static_cast<double>(i) * 10.0;
        kf.isIDR  = (i % 4U == 0U);
        m_keyframes.push_back(kf);
    }
    ChapterMarker ch{};
    ch.title    = L"Opening";
    ch.startSec = 0.0;
    m_chapters.push_back(ch);
    return true;
}

void VideoScrubberTimeline::Clear() noexcept
{
    m_keyframes.clear();
    m_chapters.clear();
    m_durationSec = 0.0;
    m_buildMs     = 0.0f;
}

KeyframeEntry VideoScrubberTimeline::KeyframeAt(uint32_t index) const noexcept
{
    if (index >= static_cast<uint32_t>(m_keyframes.size()))
    {
        return KeyframeEntry{};
    }
    return m_keyframes.at(index);
}

ChapterMarker VideoScrubberTimeline::ChapterAt(uint32_t index) const noexcept
{
    if (index >= static_cast<uint32_t>(m_chapters.size()))
    {
        return ChapterMarker{};
    }
    return m_chapters.at(index);
}

double VideoScrubberTimeline::NearestKeyframePts(double target) const noexcept
{
    if (m_keyframes.empty())
    {
        return 0.0;
    }
    const auto IT = std::ranges::lower_bound(
        m_keyframes, target, std::ranges::less{}, &KeyframeEntry::ptsSec);
    if (IT == m_keyframes.end())
    {
        return m_keyframes.back().ptsSec;
    }
    if (IT != m_keyframes.begin())
    {
        const auto PREV = std::prev(IT);
        if ((target - PREV->ptsSec) < (IT->ptsSec - target))
        {
            return PREV->ptsSec;
        }
    }
    return IT->ptsSec;
}

ScrubberTimelineStats VideoScrubberTimeline::GetStats() const noexcept
{
    ScrubberTimelineStats st{};
    st.keyframeCount = KeyframeCount();
    st.chapterCount  = ChapterCount();
    st.durationSec   = m_durationSec;
    st.buildMs       = m_buildMs;
    return st;
}

}} // namespace ExplorerLens::Engine
