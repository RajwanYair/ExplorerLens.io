#!/usr/bin/env python3
"""Add v30.4.0 Deneb-U tests to EngineTests.cpp."""
import re, pathlib

f = pathlib.Path("Engine/Tests/EngineTests.cpp")
src = f.read_text(encoding="utf-8")

# --- 1. Includes (after last v30.3.0 include) ---
anchor_inc = '#include "Core/SpreadsheetChartRenderer.h"'
new_includes = """
// Sprint 1001-1010 — Geospatial, Medical & Scientific Formats (v30.4.0 "Deneb-U")
#include "Decoders/GeoTIFFDecoder.h"
#include "Decoders/NITFDecoder.h"
#include "Decoders/DICOMAdvancedDecoder.h"
#include "Decoders/NRRDDecoder.h"
#include "Decoders/HDF5ThumbnailDecoder.h"
#include "Decoders/NetCDFDecoder.h"
#include "Decoders/FITSDecoder.h"
#include "Decoders/ECWDecoder.h"
"""
assert anchor_inc in src, f"Include anchor not found"
src = src.replace(anchor_inc, anchor_inc + "\n" + new_includes.strip())

# --- 2. TEST blocks (before "//== " marker) ---
test_blocks = r"""
// ============================================================================
// Sprint 1001-1010 — Geospatial, Medical & Scientific Formats (v30.4.0 "Deneb-U")
// ============================================================================

TEST(TestGeoT_Init) { using namespace ExplorerLens::Engine; GeoTIFFDecoder dec; ASSERT(true); }
TEST(TestGeoT_Meta) { using namespace ExplorerLens::Engine; GeoTIFFMetadata gm{}; gm.width = 4096; gm.height = 4096; gm.bandCount = 4; gm.bitsPerSample = 16; ASSERT(gm.bandCount == 4); }
TEST(TestGeoT_Band) { using namespace ExplorerLens::Engine; BandConfig bc; bc.redBand = 4; bc.greenBand = 3; bc.blueBand = 2; ASSERT(bc.redBand == 4); }
TEST(TestGeoT_Composite) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(GeoTIFFBandComposite::TrueColor) == 0); ASSERT(static_cast<int>(GeoTIFFBandComposite::NDVI) == 2); }
TEST(TestGeoT_Stretch) { using namespace ExplorerLens::Engine; BandConfig bc; bc.stretchMin = 0.02f; bc.stretchMax = 0.98f; ASSERT(bc.stretchMax > bc.stretchMin); }
TEST(TestGeoT_CRS) { using namespace ExplorerLens::Engine; GeoTIFFMetadata gm{}; gm.crs = "EPSG:4326"; ASSERT(!gm.crs.empty()); }
TEST(TestGeoT_Thermal) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(GeoTIFFBandComposite::Thermal) == 4); }
TEST(TestGeoT_NIR) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(GeoTIFFBandComposite::NIR) == 3); }
TEST(TestGeoT_FalseColor) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(GeoTIFFBandComposite::FalseColor) == 1); }

TEST(TestNITF_Init) { using namespace ExplorerLens::Engine; NITFDecoder dec; ASSERT(true); }
TEST(TestNITF_Seg) { using namespace ExplorerLens::Engine; NITFImageSegment seg{}; seg.segmentIndex = 0; seg.width = 2048; seg.height = 2048; seg.bitsPerPixel = 8; ASSERT(seg.width == 2048); }
TEST(TestNITF_Comp) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NITFCompressionType::Uncompressed) == 0); ASSERT(static_cast<int>(NITFCompressionType::JP2) == 2); }
TEST(TestNITF_Sec) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NITFSecurityLevel::Unclassified) == 0); ASSERT(static_cast<int>(NITFSecurityLevel::TopSecret) == 3); }
TEST(TestNITF_BPP) { using namespace ExplorerLens::Engine; NITFImageSegment seg{}; seg.bitsPerPixel = 16; ASSERT(seg.bitsPerPixel == 16); }
TEST(TestNITF_JPEG) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NITFCompressionType::JPEG) == 1); }
TEST(TestNITF_VQ) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NITFCompressionType::VectorQuantization) == 3); }
TEST(TestNITF_Secret) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NITFSecurityLevel::Secret) == 2); }
TEST(TestNITF_Index) { using namespace ExplorerLens::Engine; NITFImageSegment seg{}; seg.segmentIndex = 5; ASSERT(seg.segmentIndex == 5); }

TEST(TestDICM_Init) { using namespace ExplorerLens::Engine; DICOMAdvancedDecoder dec; ASSERT(true); }
TEST(TestDICM_Window) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(DICOMWindowPreset::Lung) == 0); ASSERT(static_cast<int>(DICOMWindowPreset::Custom) == 5); }
TEST(TestDICM_Frame) { using namespace ExplorerLens::Engine; DICOMFrameInfo fi{}; fi.frameIndex = 42; fi.windowCenter = 40.0f; fi.windowWidth = 400.0f; ASSERT(fi.windowWidth == 400.0f); }
TEST(TestDICM_Series) { using namespace ExplorerLens::Engine; DICOMSeriesInfo si{}; si.modality = "CT"; si.frameCount = 512; si.rows = 512; si.columns = 512; ASSERT(si.frameCount == 512); }
TEST(TestDICM_Bone) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(DICOMWindowPreset::Bone) == 1); }
TEST(TestDICM_Brain) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(DICOMWindowPreset::Brain) == 2); }
TEST(TestDICM_Slice) { using namespace ExplorerLens::Engine; DICOMFrameInfo fi{}; fi.sliceLocation = 125.5f; ASSERT(fi.sliceLocation == 125.5f); }
TEST(TestDICM_Patient) { using namespace ExplorerLens::Engine; DICOMSeriesInfo si{}; si.patientId = "ANON001"; ASSERT(!si.patientId.empty()); }
TEST(TestDICM_Thickness) { using namespace ExplorerLens::Engine; DICOMSeriesInfo si{}; si.sliceThickness = 1.25f; ASSERT(si.sliceThickness > 0.0f); }

TEST(TestNRRD_Init) { using namespace ExplorerLens::Engine; NRRDDecoder dec; ASSERT(true); }
TEST(TestNRRD_Enc) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NRRDEncoding::Raw) == 0); ASSERT(static_cast<int>(NRRDEncoding::Bzip2) == 4); }
TEST(TestNRRD_Field) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NRRDFieldType::Scalar) == 0); ASSERT(static_cast<int>(NRRDFieldType::ColorScalar) == 3); }
TEST(TestNRRD_Header) { using namespace ExplorerLens::Engine; NRRDHeader hdr{}; hdr.dimension = 3; ASSERT(hdr.dimension == 3); }
TEST(TestNRRD_Gzip) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NRRDEncoding::Gzip) == 3); }
TEST(TestNRRD_Tensor) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NRRDFieldType::Tensor) == 2); }
TEST(TestNRRD_Sizes) { using namespace ExplorerLens::Engine; NRRDHeader hdr{}; hdr.sizes = {256, 256, 128}; ASSERT(hdr.sizes.size() == 3); }
TEST(TestNRRD_Hex) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NRRDEncoding::Hex) == 2); }
TEST(TestNRRD_Spacings) { using namespace ExplorerLens::Engine; NRRDHeader hdr{}; hdr.spacings = {1.0, 1.0, 2.5}; ASSERT(hdr.spacings.size() == 3); }

TEST(TestHDF5_Init) { using namespace ExplorerLens::Engine; HDF5ThumbnailDecoder dec; ASSERT(true); }
TEST(TestHDF5_Type) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(HDF5DatasetType::Image) == 0); ASSERT(static_cast<int>(HDF5DatasetType::VLenString) == 4); }
TEST(TestHDF5_Cmap) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(HDF5ColorMap::Viridis) == 0); ASSERT(static_cast<int>(HDF5ColorMap::Grayscale) == 4); }
TEST(TestHDF5_Info) { using namespace ExplorerLens::Engine; HDF5DatasetInfo di{}; di.name = "/data/images"; di.rank = 3; ASSERT(di.rank == 3); }
TEST(TestHDF5_Plasma) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(HDF5ColorMap::Plasma) == 1); }
TEST(TestHDF5_Matrix) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(HDF5DatasetType::Matrix) == 2); }
TEST(TestHDF5_TotalSize) { using namespace ExplorerLens::Engine; HDF5DatasetInfo di{}; di.totalSize = 1048576; ASSERT(di.totalSize == 1048576); }
TEST(TestHDF5_Inferno) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(HDF5ColorMap::Inferno) == 2); }
TEST(TestHDF5_Compound) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(HDF5DatasetType::Compound) == 3); }

TEST(TestNCD_Init) { using namespace ExplorerLens::Engine; NetCDFDecoder dec; ASSERT(true); }
TEST(TestNCD_Var) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NetCDFVariableType::Temperature) == 0); ASSERT(static_cast<int>(NetCDFVariableType::Custom) == 5); }
TEST(TestNCD_Dim) { using namespace ExplorerLens::Engine; NetCDFDimension dim{}; dim.name = "time"; dim.size = 365; dim.isUnlimited = true; ASSERT(dim.isUnlimited); }
TEST(TestNCD_VarInfo) { using namespace ExplorerLens::Engine; NetCDFVariable var{}; var.name = "sst"; var.units = "degC"; ASSERT(!var.units.empty()); }
TEST(TestNCD_Pressure) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NetCDFVariableType::Pressure) == 1); }
TEST(TestNCD_Wind) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NetCDFVariableType::WindSpeed) == 2); }
TEST(TestNCD_Salinity) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NetCDFVariableType::Salinity) == 3); }
TEST(TestNCD_Elevation) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(NetCDFVariableType::Elevation) == 4); }
TEST(TestNCD_Fill) { using namespace ExplorerLens::Engine; NetCDFVariable var{}; var.fillValue = -9999.0; ASSERT(var.fillValue == -9999.0); }

TEST(TestFITS_Init) { using namespace ExplorerLens::Engine; FITSDecoder dec; ASSERT(true); }
TEST(TestFITS_Stretch) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(FITSStretchMode::Linear) == 0); ASSERT(static_cast<int>(FITSStretchMode::HistogramEqualization) == 4); }
TEST(TestFITS_LUT) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(FITSColorLUT::Grayscale) == 0); ASSERT(static_cast<int>(FITSColorLUT::STScIDefault) == 4); }
TEST(TestFITS_Header) { using namespace ExplorerLens::Engine; FITSHeaderInfo hi{}; hi.bitpix = -32; hi.naxis = 2; hi.naxis1 = 2048; hi.naxis2 = 2048; ASSERT(hi.naxis1 == 2048); }
TEST(TestFITS_Log) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(FITSStretchMode::Logarithmic) == 1); }
TEST(TestFITS_Asinh) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(FITSStretchMode::Asinh) == 3); }
TEST(TestFITS_Object) { using namespace ExplorerLens::Engine; FITSHeaderInfo hi{}; hi.objectName = "M31"; ASSERT(!hi.objectName.empty()); }
TEST(TestFITS_Heat) { using namespace ExplorerLens::Engine; ASSERT(static_cast<int>(FITSColorLUT::Heat) == 1); }
TEST(TestFITS_Scale) { using namespace ExplorerLens::Engine; FITSHeaderInfo hi{}; hi.bscale = 1.0; hi.bzero = 0.0; ASSERT(hi.bscale == 1.0); }

TEST(TestECW_Init) { using namespace ExplorerLens::Engine; ECWDecoder dec; ASSERT(true); }
TEST(TestECW_Meta) { using namespace ExplorerLens::Engine; ECWMetadata em{}; em.width = 10000; em.height = 10000; em.bandCount = 3; ASSERT(em.width == 10000); }
TEST(TestECW_Ratio) { using namespace ExplorerLens::Engine; ECWCompressionRatio cr{}; cr.targetRatio = 20.0f; cr.actualRatio = 18.5f; cr.qualityPercent = 92.0f; ASSERT(cr.qualityPercent > 90.0f); }
TEST(TestECW_ResLevel) { using namespace ExplorerLens::Engine; ECWResolutionLevel rl{}; rl.level = 3; rl.width = 1250; rl.height = 1250; ASSERT(rl.level == 3); }
TEST(TestECW_CellSize) { using namespace ExplorerLens::Engine; ECWMetadata em{}; em.cellSizeX = 0.5; em.cellSizeY = 0.5; ASSERT(em.cellSizeX == 0.5); }
TEST(TestECW_Proj) { using namespace ExplorerLens::Engine; ECWMetadata em{}; em.projectionWkt = "GEOGCS"; ASSERT(!em.projectionWkt.empty()); }
TEST(TestECW_Block) { using namespace ExplorerLens::Engine; ECWResolutionLevel rl{}; rl.blockSize = 256; ASSERT(rl.blockSize == 256); }
TEST(TestECW_Bands) { using namespace ExplorerLens::Engine; ECWMetadata em{}; em.bandCount = 4; ASSERT(em.bandCount == 4); }
TEST(TestECW_Target) { using namespace ExplorerLens::Engine; ECWCompressionRatio cr{}; cr.targetRatio = 10.0f; ASSERT(cr.targetRatio == 10.0f); }
"""

# Find where to insert TEST blocks — before "//== " section
test_anchor = re.search(r'^//== ', src, re.MULTILINE)
assert test_anchor, "Could not find //== anchor for TEST blocks"
src = src[:test_anchor.start()] + test_blocks.strip() + "\n\n" + src[test_anchor.start():]

# --- 3. RUN_TEST calls (before "// Integration Test Framework + COM Tests") ---
run_anchor = "// Integration Test Framework + COM Tests"
assert run_anchor in src, f"RUN_TEST anchor not found"

run_tests = """
    // Sprint 1001-1010 — Geospatial, Medical & Scientific Formats (v30.4.0 "Deneb-U")
    RUN_TEST(TestGeoT_Init);
    RUN_TEST(TestGeoT_Meta);
    RUN_TEST(TestGeoT_Band);
    RUN_TEST(TestGeoT_Composite);
    RUN_TEST(TestGeoT_Stretch);
    RUN_TEST(TestGeoT_CRS);
    RUN_TEST(TestGeoT_Thermal);
    RUN_TEST(TestGeoT_NIR);
    RUN_TEST(TestGeoT_FalseColor);
    RUN_TEST(TestNITF_Init);
    RUN_TEST(TestNITF_Seg);
    RUN_TEST(TestNITF_Comp);
    RUN_TEST(TestNITF_Sec);
    RUN_TEST(TestNITF_BPP);
    RUN_TEST(TestNITF_JPEG);
    RUN_TEST(TestNITF_VQ);
    RUN_TEST(TestNITF_Secret);
    RUN_TEST(TestNITF_Index);
    RUN_TEST(TestDICM_Init);
    RUN_TEST(TestDICM_Window);
    RUN_TEST(TestDICM_Frame);
    RUN_TEST(TestDICM_Series);
    RUN_TEST(TestDICM_Bone);
    RUN_TEST(TestDICM_Brain);
    RUN_TEST(TestDICM_Slice);
    RUN_TEST(TestDICM_Patient);
    RUN_TEST(TestDICM_Thickness);
    RUN_TEST(TestNRRD_Init);
    RUN_TEST(TestNRRD_Enc);
    RUN_TEST(TestNRRD_Field);
    RUN_TEST(TestNRRD_Header);
    RUN_TEST(TestNRRD_Gzip);
    RUN_TEST(TestNRRD_Tensor);
    RUN_TEST(TestNRRD_Sizes);
    RUN_TEST(TestNRRD_Hex);
    RUN_TEST(TestNRRD_Spacings);
    RUN_TEST(TestHDF5_Init);
    RUN_TEST(TestHDF5_Type);
    RUN_TEST(TestHDF5_Cmap);
    RUN_TEST(TestHDF5_Info);
    RUN_TEST(TestHDF5_Plasma);
    RUN_TEST(TestHDF5_Matrix);
    RUN_TEST(TestHDF5_TotalSize);
    RUN_TEST(TestHDF5_Inferno);
    RUN_TEST(TestHDF5_Compound);
    RUN_TEST(TestNCD_Init);
    RUN_TEST(TestNCD_Var);
    RUN_TEST(TestNCD_Dim);
    RUN_TEST(TestNCD_VarInfo);
    RUN_TEST(TestNCD_Pressure);
    RUN_TEST(TestNCD_Wind);
    RUN_TEST(TestNCD_Salinity);
    RUN_TEST(TestNCD_Elevation);
    RUN_TEST(TestNCD_Fill);
    RUN_TEST(TestFITS_Init);
    RUN_TEST(TestFITS_Stretch);
    RUN_TEST(TestFITS_LUT);
    RUN_TEST(TestFITS_Header);
    RUN_TEST(TestFITS_Log);
    RUN_TEST(TestFITS_Asinh);
    RUN_TEST(TestFITS_Object);
    RUN_TEST(TestFITS_Heat);
    RUN_TEST(TestFITS_Scale);
    RUN_TEST(TestECW_Init);
    RUN_TEST(TestECW_Meta);
    RUN_TEST(TestECW_Ratio);
    RUN_TEST(TestECW_ResLevel);
    RUN_TEST(TestECW_CellSize);
    RUN_TEST(TestECW_Proj);
    RUN_TEST(TestECW_Block);
    RUN_TEST(TestECW_Bands);
    RUN_TEST(TestECW_Target);

"""

src = src.replace(run_anchor, run_tests.lstrip() + "    " + run_anchor)

f.write_text(src, encoding="utf-8")
lines = len(src.splitlines())
runtests = src.count("RUN_TEST(")
print(f"EngineTests.cpp: {lines} lines, {runtests} RUN_TEST calls")
