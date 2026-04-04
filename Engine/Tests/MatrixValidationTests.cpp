// Matrix Validation Framework — GTest
#include "../Utils/QualityGates.h"
#include "GTestShim.h"

using namespace ExplorerLens::Utils;

TEST(MatrixValidationFramework, MockReportAllPassGateOpen)
{
    auto r = MatrixValidationReport::CreateMock(true);
    EXPECT_TRUE(r.gatePassed);
}

TEST(MatrixValidationFramework, MockReportAllPassRate100)
{
    auto r = MatrixValidationReport::CreateMock(true);
    EXPECT_DOUBLE_EQ(r.OverallPassRate(), 100.0);
}

TEST(MatrixValidationFramework, MockReportFailGateClosed)
{
    auto r = MatrixValidationReport::CreateMock(false);
    EXPECT_FALSE(r.gatePassed);
}

TEST(MatrixValidationFramework, MockReportHas5Cells)
{
    auto r = MatrixValidationReport::CreateMock(true);
    EXPECT_EQ(r.cells.size(), 5u);
}

TEST(MatrixValidationFramework, ValidationDomainToStringNotEmpty)
{
    EXPECT_FALSE(ToString(ValidationDomain::Decoder).empty());
}

TEST(MatrixValidationFramework, ValidationStatusToStringPass)
{
    EXPECT_EQ(ToString(ValidationStatus::Pass), std::string("PASS"));
}

TEST(MatrixValidationFramework, ValidationCaseIsPassForPass)
{
    ValidationCase c;
    c.status = ValidationStatus::Pass;
    EXPECT_TRUE(c.IsPass());
}

TEST(MatrixValidationFramework, ValidationCaseIsPassForWarning)
{
    ValidationCase c;
    c.status = ValidationStatus::Warning;
    EXPECT_TRUE(c.IsPass());
}

TEST(MatrixValidationFramework, ValidationCaseIsNotPassForFail)
{
    ValidationCase c;
    c.status = ValidationStatus::Fail;
    EXPECT_FALSE(c.IsPass());
}

TEST(MatrixValidationFramework, MatrixCellPassRate100)
{
    MatrixCell cell;
    cell.caseCount = 10;
    cell.passCount = 10;
    EXPECT_DOUBLE_EQ(cell.PassRate(), 100.0);
}

TEST(MatrixValidationFramework, GateThresholdIs95)
{
    EXPECT_DOUBLE_EQ(MatrixValidationReport::kPassGateThreshold, 95.0);
}

TEST(MatrixValidationFramework, FrameworkRunEmptyReturnsNoFailures)
{
    MatrixValidationFramework fw;
    auto r = fw.RunAll();
    EXPECT_TRUE(r.failures.empty());
}

TEST(MatrixValidationFramework, FrameworkWithPassingValidator)
{
    MatrixValidationFramework fw;
    fw.RegisterValidator(ValidationDomain::Decoder, "TestDecoder", []() {
        ValidationCase c;
        c.name = "TestCase";
        c.status = ValidationStatus::Pass;
        return std::vector<ValidationCase>{c};
    });
    auto r = fw.RunAll();
    EXPECT_EQ(r.totalPassed, 1u);
    EXPECT_EQ(r.totalCases, 1u);
}

TEST(MatrixValidationFramework, FrameworkWithFailingValidator)
{
    MatrixValidationFramework fw;
    fw.RegisterValidator(ValidationDomain::Memory, "MemTest", []() {
        ValidationCase c;
        c.name = "MemCase";
        c.status = ValidationStatus::Fail;
        return std::vector<ValidationCase>{c};
    });
    auto r = fw.RunAll();
    EXPECT_EQ(r.failures.size(), 1u);
}

TEST(MatrixValidationFramework, MatrixCellPassRateZeroWhenEmpty)
{
    MatrixCell cell;
    cell.caseCount = 0;
    cell.passCount = 0;
    EXPECT_DOUBLE_EQ(cell.PassRate(), 0.0);
}

TEST(MatrixValidationFramework, MockCellTotalMsPositive)
{
    auto r = MatrixValidationReport::CreateMock(true);
    (void)r;
    for (const auto& c : r.cells) {
        (void)c;
        EXPECT_GT(c.totalMs, 0.0);
    }
}

TEST(MatrixValidationFramework, FrameworkRunAllGatePassed)
{
    MatrixValidationFramework fw;
    for (int i = 0; i < 5; ++i) {
        fw.RegisterValidator(ValidationDomain::Decoder, "D" + std::to_string(i), []() {
            ValidationCase c;
            c.name = "C";
            c.status = ValidationStatus::Pass;
            return std::vector<ValidationCase>{c};
        });
    }
    auto r = fw.RunAll();
    EXPECT_TRUE(r.gatePassed);
}
