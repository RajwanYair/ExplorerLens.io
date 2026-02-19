// Sprint 166 — Zero-Copy Pipeline — GTest
#include <gtest/gtest.h>
#include "Pipeline/ZeroCopyPipeline.h"

using namespace DarkThumbs::Pipeline;

TEST(ZeroCopyPipeline, PinnedBufferIsZeroCopyCapable) {
    EXPECT_TRUE(IsZeroCopyCapable(BufferOrigin::PinnedVirtual));
}

TEST(ZeroCopyPipeline, MappedFileIsZeroCopyCapable) {
    EXPECT_TRUE(IsZeroCopyCapable(BufferOrigin::MappedFile));
}

TEST(ZeroCopyPipeline, HeapMallocNotZeroCopy) {
    EXPECT_FALSE(IsZeroCopyCapable(BufferOrigin::HeapMalloc));
}

TEST(ZeroCopyPipeline, AllocatePinnedReturnsPinnedOrigin) {
    auto buf = ZeroCopyPipeline::AllocatePinned(4096);
    EXPECT_EQ(buf.origin, BufferOrigin::PinnedVirtual);
    EXPECT_EQ(buf.sizeBytes, 4096u);
}

TEST(ZeroCopyPipeline, WrapMappedFileReadOnly) {
    uint8_t data[64] = {};
    auto buf = ZeroCopyPipeline::WrapMappedFile(data, 64);
    EXPECT_TRUE(buf.isReadOnly);
    EXPECT_EQ(buf.origin, BufferOrigin::MappedFile);
}

TEST(ZeroCopyPipeline, InvalidBufferCannotUpload) {
    ZeroCopyBuffer buf;
    EXPECT_FALSE(buf.CanUploadToGPU());
}

TEST(ZeroCopyPipeline, UploadWithNullDescriptorFails) {
    GPUUploadDescriptor desc;
    ZeroCopyStats stats;
    EXPECT_FALSE(ZeroCopyPipeline::UploadToGPU(desc, stats));
}

TEST(ZeroCopyPipeline, UploadPinnedCountsAsZeroCopy) {
    uint8_t data[256] = {};
    auto buf = ZeroCopyPipeline::WrapMappedFile(data, 256);
    GPUUploadDescriptor desc;
    desc.srcBuffer = &buf;
    desc.widthPx = 8; desc.heightPx = 8; desc.rowPitchBytes = 32;
    ZeroCopyStats stats;
    ZeroCopyPipeline::UploadToGPU(desc, stats);
    EXPECT_GT(stats.bytesHandedOff + stats.bytesCopied, 0u);
}

TEST(ZeroCopyPipeline, ScatterGatherIsValidOnCorrectTotal) {
    ScatterGatherReadback sg;
    sg.totalBytes = 100;
    sg.segments.push_back({ nullptr, 0, 100 });
    EXPECT_TRUE(sg.IsValid());
}

TEST(ZeroCopyPipeline, ScatterGatherInvalidOnMismatch) {
    ScatterGatherReadback sg;
    sg.totalBytes = 200;
    sg.segments.push_back({ nullptr, 0, 100 });
    EXPECT_FALSE(sg.IsValid());
}

TEST(ZeroCopyPipeline, ZeroCopyStatsIsZeroCopy) {
    ZeroCopyStats stats;
    stats.bytesCopied = 0;
    stats.bytesHandedOff = 512;
    EXPECT_TRUE(stats.IsZeroCopy());
}

TEST(ZeroCopyPipeline, ZeroCopyStatsNotZeroCopyWhenCopied) {
    ZeroCopyStats stats;
    stats.bytesCopied = 512;
    stats.bytesHandedOff = 0;
    EXPECT_FALSE(stats.IsZeroCopy());
}

TEST(ZeroCopyPipeline, BufferOriginEnumD3D11Staging) {
    EXPECT_EQ(static_cast<uint32_t>(BufferOrigin::D3D11Staging), 3u);
}
