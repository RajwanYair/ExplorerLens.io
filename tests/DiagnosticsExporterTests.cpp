#include <gtest/gtest.h>
#include "Core/DiagnosticsExporter.h"
using namespace ExplorerLens::Core;

TEST(Sprint144_Diag, CategoryName_Coverage) {
    EXPECT_STREQ(DiagCategoryName(DiagCategory::SystemInfo), "SystemInfo");
    EXPECT_STREQ(DiagCategoryName(DiagCategory::ETWLogs), "ETWLogs");
    EXPECT_STREQ(DiagCategoryName(DiagCategory::GPUDiagnostics), "GPUDiagnostics");
}
TEST(Sprint144_Diag, DiagEntry_Create) {
    auto e = DiagEntry::Create(DiagCategory::ErrorLogs, "errors.txt", "some error");
    EXPECT_EQ(e.category, DiagCategory::ErrorLogs);
    EXPECT_EQ(e.filename, "errors.txt");
    EXPECT_EQ(e.sizeBytes, 10u);
}
TEST(Sprint144_Diag, ExportStatusName_Coverage) {
    EXPECT_STREQ(ExportStatusName(ExportStatus::Success), "Success");
    EXPECT_STREQ(ExportStatusName(ExportStatus::PartialSuccess), "PartialSuccess");
    EXPECT_STREQ(ExportStatusName(ExportStatus::NoData), "NoData");
}
TEST(Sprint144_Diag, ExportResult_Summary) {
    ExportResult r;
    r.status = ExportStatus::Success;
    r.fileCount = 5; r.totalSizeBytes = 2048;
    EXPECT_NE(r.Summary().find("Success"), std::string::npos);
    EXPECT_NE(r.Summary().find("files=5"), std::string::npos);
}
TEST(Sprint144_Diag, Exporter_AddEntries) {
    auto exp = DiagnosticsExporter::Create();
    exp.AddSystemInfo("OS: Windows 11");
    exp.AddDecoderHealth("{\"status\":\"ok\"}");
    exp.AddErrorLog("No errors");
    EXPECT_EQ(exp.EntryCount(), 3u);
}
TEST(Sprint144_Diag, Exporter_ExportSuccess) {
    auto exp = DiagnosticsExporter::Create();
    exp.AddSystemInfo("Test system info");
    auto result = exp.Export();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.fileCount, 1u);
    EXPECT_GT(result.totalSizeBytes, 0u);
}
TEST(Sprint144_Diag, Exporter_ExportNoData) {
    auto exp = DiagnosticsExporter::Create();
    auto result = exp.Export();
    EXPECT_EQ(result.status, ExportStatus::NoData);
    EXPECT_FALSE(result.IsSuccess());
}
TEST(Sprint144_Diag, Exporter_FilterSensitive) {
    auto cfg = DiagExportConfig::Default();
    cfg.includeSensitive = false;
    auto exp = DiagnosticsExporter::Create(cfg);
    DiagEntry sensitive = DiagEntry::Create(DiagCategory::Configuration, "config.txt", "secrets");
    sensitive.sensitive = true;
    exp.AddEntry(sensitive);
    exp.AddSystemInfo("Safe data");
    auto filtered = exp.FilteredEntries();
    EXPECT_EQ(filtered.size(), 1u);
    auto result = exp.Export();
    EXPECT_EQ(result.status, ExportStatus::PartialSuccess);
    EXPECT_EQ(result.skippedCount, 1u);
}
TEST(Sprint144_Diag, Exporter_FilterETW) {
    auto cfg = DiagExportConfig::Minimal();
    auto exp = DiagnosticsExporter::Create(cfg);
    exp.AddEntry(DiagEntry::Create(DiagCategory::ETWLogs, "etw.log", "etw data"));
    exp.AddSystemInfo("sys");
    auto filtered = exp.FilteredEntries();
    EXPECT_EQ(filtered.size(), 1u); // ETW excluded
}
TEST(Sprint144_Diag, Config_Presets) {
    auto def = DiagExportConfig::Default();
    auto min = DiagExportConfig::Minimal();
    auto full = DiagExportConfig::Full();
    EXPECT_TRUE(def.includeETW);
    EXPECT_FALSE(min.includeETW);
    EXPECT_TRUE(full.includeSensitive);
}
TEST(Sprint144_Diag, AnonymizePath) {
    std::string path = "C:\\Users\\john\\Documents\\test.txt";
    auto anon = DiagnosticsExporter::AnonymizePath(path);
    EXPECT_NE(anon.find("<USER>"), std::string::npos);
    EXPECT_EQ(anon.find("john"), std::string::npos);
}
TEST(Sprint144_Diag, Exporter_SizeLimit) {
    auto cfg = DiagExportConfig::Default();
    cfg.maxLogSizeBytes = 5;
    auto exp = DiagnosticsExporter::Create(cfg);
    exp.AddEntry(DiagEntry::Create(DiagCategory::ErrorLogs, "big.log", "this is too large data"));
    exp.AddSystemInfo("ok");
    auto filtered = exp.FilteredEntries();
    EXPECT_EQ(filtered.size(), 1u); // big file excluded
}

