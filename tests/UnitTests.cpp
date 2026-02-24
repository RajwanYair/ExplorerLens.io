///////////////////////////////////////////////
// ExplorerLens Unit Test Suite
// Comprehensive tests for all core functionality
///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <atlbase.h>
#include <atlstr.h>
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <filesystem>

// Test harness macros
#define TEST_SUITE(name) namespace name {
#define END_TEST_SUITE() }
#define TEST_CASE(name) void name()
#define ASSERT_TRUE(expr) if (!(expr)) { std::cerr << "FAILED: " << #expr << " at line " << __LINE__ << std::endl; g_testsFailed++; } else { g_testsPass ed++; }
#define ASSERT_FALSE(expr) if ((expr)) { std::cerr << "FAILED: !" << #expr << " at line " << __LINE__ << std::endl; g_testsFailed++; } else { g_testsP assed++; }
#define ASSERT_EQUAL(a, b) if ((a) != (b)) { std::cerr << "FAILED: " << #a << " == " << #b << " at line " << __LINE__ << std::endl; g_testsFailed++; } else { g_testsPassed++; }
#define RUN_TEST(test) std::cout << "Running " << #test << "..." << std::endl; test(); std::cout << "  Completed." << std::endl;

// Global test counters
int g_testsPassed = 0;
int g_testsFailed = 0;

///////////////////////////////////////////////
// Mock implementations for isolated testing
///////////////////////////////////////////////

namespace TestMocks {
    // Mock LENSTYPE definitions (match lensArchive.h)
    #define LENSTYPE int
    #define LENSTYPE_NONE 0
    #define LENSTYPE_ZIP  1
    #define LENSTYPE_CBZ  2
    #define LENSTYPE_RAR  3
    #define LENSTYPE_CBR  4
    #define LENSTYPE_EPUB 5
    #define LENSTYPE_7Z   6
    #define LENSTYPE_CB7  7
    #define LENSTYPE_TAR  8
    #define LENSTYPE_CBT  9
    #define LENSTYPE_MOBI 10
    #define LENSTYPE_FB2  11

    // Helper function (copied from lensArchive.h for testing)
    inline BOOL StrEqual(LPCTSTR psz1, LPCTSTR psz2) {
        return (::StrCmpI(psz1, psz2) == 0);
    }

    // Function under test: IsImage
    BOOL IsImage(LPCTSTR szFile) {
        LPWSTR _e = PathFindExtension(szFile);
        // Traditional formats
        if (StrEqual(_e, _T(".bmp")))  return TRUE;
        if (StrEqual(_e, _T(".ico")))  return TRUE;
        if (StrEqual(_e, _T(".gif")))  return TRUE;
        if (StrEqual(_e, _T(".jpg")))  return TRUE;
        if (StrEqual(_e, _T(".jpe")))  return TRUE;
        if (StrEqual(_e, _T(".jfif"))) return TRUE;
        if (StrEqual(_e, _T(".jpeg"))) return TRUE;
        if (StrEqual(_e, _T(".png")))  return TRUE;
        if (StrEqual(_e, _T(".tif")))  return TRUE;
        if (StrEqual(_e, _T(".tiff"))) return TRUE;
        // Modern formats (Windows 11)
        if (StrEqual(_e, _T(".webp"))) return TRUE;
        if (StrEqual(_e, _T(".avif"))) return TRUE;
        if (StrEqual(_e, _T(".jxl")))  return TRUE;
        if (StrEqual(_e, _T(".jxr")))  return TRUE;
        if (StrEqual(_e, _T(".heif"))) return TRUE;
        if (StrEqual(_e, _T(".heic"))) return TRUE;
        return FALSE;
    }

    // Function under test: GetLENSType
    inline LENSTYPE GetLENSType(LPCTSTR szExt) {
        // Comic book archives
        if (StrEqual(szExt, _T(".cbz"))) return LENSTYPE_CBZ;
        if (StrEqual(szExt, _T(".cbr"))) return LENSTYPE_CBR;
        if (StrEqual(szExt, _T(".cb7"))) return LENSTYPE_CB7;
        if (StrEqual(szExt, _T(".cbt"))) return LENSTYPE_CBT;
        
        // Generic archives
        if (StrEqual(szExt, _T(".zip"))) return LENSTYPE_ZIP;
        if (StrEqual(szExt, _T(".rar"))) return LENSTYPE_RAR;
        if (StrEqual(szExt, _T(".7z")))  return LENSTYPE_7Z;
        if (StrEqual(szExt, _T(".tar"))) return LENSTYPE_TAR;
        
        // Ebook formats
        if (StrEqual(szExt, _T(".epub"))) return LENSTYPE_EPUB;
        if (StrEqual(szExt, _T(".mobi"))) return LENSTYPE_MOBI;
        if (StrEqual(szExt, _T(".azw")))  return LENSTYPE_MOBI;
        if (StrEqual(szExt, _T(".azw3"))) return LENSTYPE_MOBI;
        if (StrEqual(szExt, _T(".fb2")))  return LENSTYPE_FB2;
        
        // Photo archives
        if (StrEqual(szExt, _T(".phz"))) return LENSTYPE_CBZ;
        
        return LENSTYPE_NONE;
    }
}

///////////////////////////////////////////////
// Test Suite 1: Format Detection Tests
///////////////////////////////////////////////

TEST_SUITE(FormatDetectionTests)

TEST_CASE(TestGetLENSType_ComicArchives) {
    std::cout << "  Testing comic book archive format detection..." << std::endl;
    
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".cbz")), LENSTYPE_CBZ);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".cbr")), LENSTYPE_CBR);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".cb7")), LENSTYPE_CB7);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".cbt")), LENSTYPE_CBT);
    
    // Case insensitive
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".CBZ")), LENSTYPE_CBZ);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".CbR")), LENSTYPE_CBR);
}

TEST_CASE(TestGetLENSType_GenericArchives) {
    std::cout << "  Testing generic archive format detection..." << std::endl;
    
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".zip")), LENSTYPE_ZIP);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".rar")), LENSTYPE_RAR);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".7z")), LENSTYPE_7Z);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".tar")), LENSTYPE_TAR);
    
    // Case insensitive
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".ZIP")), LENSTYPE_ZIP);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".Tar")), LENSTYPE_TAR);
}

TEST_CASE(TestGetLENSType_EbookFormats) {
    std::cout << "  Testing ebook format detection..." << std::endl;
    
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".epub")), LENSTYPE_EPUB);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".mobi")), LENSTYPE_MOBI);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".azw")), LENSTYPE_MOBI);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".azw3")), LENSTYPE_MOBI);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".fb2")), LENSTYPE_FB2);
    
    // Case insensitive
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".EPUB")), LENSTYPE_EPUB);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".Mobi")), LENSTYPE_MOBI);
}

TEST_CASE(TestGetLENSType_PhotoArchives) {
    std::cout << "  Testing photo archive format detection..." << std::endl;
    
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".phz")), LENSTYPE_CBZ);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".PHZ")), LENSTYPE_CBZ);
}

TEST_CASE(TestGetLENSType_UnsupportedFormats) {
    std::cout << "  Testing unsupported format detection..." << std::endl;
    
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".txt")), LENSTYPE_NONE);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".pdf")), LENSTYPE_NONE);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".doc")), LENSTYPE_NONE);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".exe")), LENSTYPE_NONE);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T("")), LENSTYPE_NONE);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T("noextension")), LENSTYPE_NONE);
}

TEST_CASE(TestGetLENSType_EdgeCases) {
    std::cout << "  Testing edge cases..." << std::endl;
    
    // Multiple dots
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".backup.cbz")), LENSTYPE_CBZ);
    
    // Single character before extension
    ASSERT_EQUAL(TestMocks::GetLENSType(_T("a.zip")), LENSTYPE_ZIP);
}

END_TEST_SUITE()

///////////////////////////////////////////////
// Test Suite 2: Image Format Detection Tests
///////////////////////////////////////////////

TEST_SUITE(ImageFormatTests)

TEST_CASE(TestIsImage_TraditionalFormats) {
    std::cout << "  Testing traditional image format detection..." << std::endl;
    
    ASSERT_TRUE(TestMocks::IsImage(_T("image.bmp")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.ico")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.gif")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.jpg")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.jpe")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.jfif")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.jpeg")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.png")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.tif")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.tiff")));
}

TEST_CASE(TestIsImage_ModernFormats) {
    std::cout << "  Testing modern image format detection (Windows 11)..." << std::endl;
    
    ASSERT_TRUE(TestMocks::IsImage(_T("image.webp")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.avif")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.jxl")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.jxr")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.heif")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.heic")));
}

TEST_CASE(TestIsImage_CaseInsensitive) {
    std::cout << "  Testing case insensitive image detection..." << std::endl;
    
    ASSERT_TRUE(TestMocks::IsImage(_T("IMAGE.JPG")));
    ASSERT_TRUE(TestMocks::IsImage(_T("Image.Png")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image.WEBP")));
    ASSERT_TRUE(TestMocks::IsImage(_T("IMAGE.AVIF")));
}

TEST_CASE(TestIsImage_WithPaths) {
    std::cout << "  Testing image detection with full paths..." << std::endl;
    
    ASSERT_TRUE(TestMocks::IsImage(_T("C:\\folder\\image.jpg")));
    ASSERT_TRUE(TestMocks::IsImage(_T("\\\\network\\share\\photo.png")));
    ASSERT_TRUE(TestMocks::IsImage(_T("subfolder/another/pic.webp")));
    ASSERT_TRUE(TestMocks::IsImage(_T("./relative/path/img.avif")));
}

TEST_CASE(TestIsImage_NonImageFiles) {
    std::cout << "  Testing non-image file rejection..." << std::endl;
    
    ASSERT_FALSE(TestMocks::IsImage(_T("document.txt")));
    ASSERT_FALSE(TestMocks::IsImage(_T("archive.zip")));
    ASSERT_FALSE(TestMocks::IsImage(_T("video.mp4")));
    ASSERT_FALSE(TestMocks::IsImage(_T("audio.mp3")));
    ASSERT_FALSE(TestMocks::IsImage(_T("script.exe")));
}

TEST_CASE(TestIsImage_EdgeCases) {
    std::cout << "  Testing image detection edge cases..." << std::endl;
    
    // Multiple extensions
    ASSERT_TRUE(TestMocks::IsImage(_T("file.backup.jpg")));
    
    // Special characters in filename
    ASSERT_TRUE(TestMocks::IsImage(_T("my photo (1).png")));
    ASSERT_TRUE(TestMocks::IsImage(_T("image_2024-01-15.webp")));
    
    // Very short name
    ASSERT_TRUE(TestMocks::IsImage(_T("a.jpg")));
    
    // No extension
    ASSERT_FALSE(TestMocks::IsImage(_T("noextension")));
    ASSERT_FALSE(TestMocks::IsImage(_T("")));
}

END_TEST_SUITE()

///////////////////////////////////////////////
// Test Suite 3: String Utility Tests
///////////////////////////////////////////////

TEST_SUITE(StringUtilityTests)

TEST_CASE(TestStrEqual_BasicComparison) {
    std::cout << "  Testing basic string comparison..." << std::endl;
    
    ASSERT_TRUE(TestMocks::StrEqual(_T("test"), _T("test")));
    ASSERT_TRUE(TestMocks::StrEqual(_T(""), _T("")));
    ASSERT_FALSE(TestMocks::StrEqual(_T("test"), _T("other")));
}

TEST_CASE(TestStrEqual_CaseInsensitive) {
    std::cout << "  Testing case insensitive comparison..." << std::endl;
    
    ASSERT_TRUE(TestMocks::StrEqual(_T("Test"), _T("test")));
    ASSERT_TRUE(TestMocks::StrEqual(_T("TEST"), _T("test")));
    ASSERT_TRUE(TestMocks::StrEqual(_T("TeSt"), _T("tEsT")));
    ASSERT_TRUE(TestMocks::StrEqual(_T(".ZIP"), _T(".zip")));
}

TEST_CASE(TestStrEqual_SpecialCharacters) {
    std::cout << "  Testing special character comparison..." << std::endl;
    
    ASSERT_TRUE(TestMocks::StrEqual(_T(".cbz"), _T(".CBZ")));
    ASSERT_TRUE(TestMocks::StrEqual(_T("file_name.ext"), _T("FILE_NAME.EXT")));
    ASSERT_FALSE(TestMocks::StrEqual(_T(".cbz"), _T(".cbr")));
}

END_TEST_SUITE()

///////////////////////////////////////////////
// Test Suite 4: Format Coverage Tests
///////////////////////////////////////////////

TEST_SUITE(FormatCoverageTests)

TEST_CASE(TestAllArchiveFormatsSupported) {
    std::cout << "  Verifying all 12 archive formats are detected..." << std::endl;
    
    std::vector<std::pair<std::wstring, int>> formats = {
        {L".zip", LENSTYPE_ZIP},
        {L".cbz", LENSTYPE_CBZ},
        {L".rar", LENSTYPE_RAR},
        {L".cbr", LENSTYPE_CBR},
        {L".epub", LENSTYPE_EPUB},
        {L".7z", LENSTYPE_7Z},
        {L".cb7", LENSTYPE_CB7},
        {L".tar", LENSTYPE_TAR},
        {L".cbt", LENSTYPE_CBT},
        {L".mobi", LENSTYPE_MOBI},
        {L".azw", LENSTYPE_MOBI},
        {L".azw3", LENSTYPE_MOBI},
        {L".fb2", LENSTYPE_FB2},
        {L".phz", LENSTYPE_CBZ}
    };
    
    int supportedCount = 0;
    for (const auto& fmt : formats) {
        LENSTYPE result = TestMocks::GetLENSType(fmt.first.c_str());
        if (result != LENSTYPE_NONE) {
            supportedCount++;
            ASSERT_EQUAL(result, fmt.second);
        }
    }
    
    std::cout << "    Supported formats: " << supportedCount << " / " << formats.size() << std::endl;
    ASSERT_TRUE(supportedCount >= 12); // At least 12 unique type detections
}

TEST_CASE(TestAllImageFormatsSupported) {
    std::cout << "  Verifying all 16 image formats are detected..." << std::endl;
    
    std::vector<std::wstring> imageFormats = {
        L"test.bmp", L"test.ico", L"test.gif",
        L"test.jpg", L"test.jpe", L"test.jfif", L"test.jpeg",
        L"test.png", L"test.tif", L"test.tiff",
        L"test.webp", L"test.avif", L"test.jxl",
        L"test.jxr", L"test.heif", L"test.heic"
    };
    
    int supportedCount = 0;
    for (const auto& img : imageFormats) {
        if (TestMocks::IsImage(img.c_str())) {
            supportedCount++;
        }
    }
    
    std::cout << "    Supported image formats: " << supportedCount << " / " << imageFormats.size() << std::endl;
    ASSERT_EQUAL(supportedCount, 16);
}

TEST_CASE(TestNoFormatCollisions) {
    std::cout << "  Testing for format type collisions..." << std::endl;
    
    // Ensure each format type is unique
    ASSERT_TRUE(LENSTYPE_NONE != LENSTYPE_ZIP);
    ASSERT_TRUE(LENSTYPE_ZIP != LENSTYPE_CBZ);
    ASSERT_TRUE(LENSTYPE_RAR != LENSTYPE_CBR);
    ASSERT_TRUE(LENSTYPE_7Z != LENSTYPE_CB7);
    ASSERT_TRUE(LENSTYPE_TAR != LENSTYPE_CBT);
    ASSERT_TRUE(LENSTYPE_MOBI != LENSTYPE_FB2);
    ASSERT_TRUE(LENSTYPE_EPUB != LENSTYPE_MOBI);
    
    // Verify type value ranges
    ASSERT_TRUE(LENSTYPE_NONE == 0);
    ASSERT_TRUE(LENSTYPE_ZIP == 1);
    ASSERT_TRUE(LENSTYPE_FB2 == 11);
}

END_TEST_SUITE()

///////////////////////////////////////////////
// Test Suite 5: Regression Tests
///////////////////////////////////////////////

TEST_SUITE(RegressionTests)

TEST_CASE(TestBackwardCompatibility_OriginalFormats) {
    std::cout << "  Testing backward compatibility with original 6 formats..." << std::endl;
    
    // Original ExplorerLens supported these 6 formats
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".zip")), LENSTYPE_ZIP);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".cbz")), LENSTYPE_CBZ);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".rar")), LENSTYPE_RAR);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".cbr")), LENSTYPE_CBR);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".epub")), LENSTYPE_EPUB);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".phz")), LENSTYPE_CBZ);
}

TEST_CASE(TestBackwardCompatibility_OriginalImages) {
    std::cout << "  Testing backward compatibility with original 10 image formats..." << std::endl;
    
    // Original supported formats
    ASSERT_TRUE(TestMocks::IsImage(_T("test.bmp")));
    ASSERT_TRUE(TestMocks::IsImage(_T("test.gif")));
    ASSERT_TRUE(TestMocks::IsImage(_T("test.jpg")));
    ASSERT_TRUE(TestMocks::IsImage(_T("test.jpeg")));
    ASSERT_TRUE(TestMocks::IsImage(_T("test.png")));
    ASSERT_TRUE(TestMocks::IsImage(_T("test.tif")));
    ASSERT_TRUE(TestMocks::IsImage(_T("test.tiff")));
}

TEST_CASE(TestEnhancement_NewArchiveFormats) {
    std::cout << "  Testing enhancement: 6 new archive formats..." << std::endl;
    
    // New formats added in Windows 11 modernization
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".7z")), LENSTYPE_7Z);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".cb7")), LENSTYPE_CB7);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".tar")), LENSTYPE_TAR);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".cbt")), LENSTYPE_CBT);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".mobi")), LENSTYPE_MOBI);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".fb2")), LENSTYPE_FB2);
}

TEST_CASE(TestEnhancement_NewImageFormats) {
    std::cout << "  Testing enhancement: 6 new image formats..." << std::endl;
    
    // New modern formats for Windows 11
    ASSERT_TRUE(TestMocks::IsImage(_T("test.webp")));
    ASSERT_TRUE(TestMocks::IsImage(_T("test.avif")));
    ASSERT_TRUE(TestMocks::IsImage(_T("test.jxl")));
    ASSERT_TRUE(TestMocks::IsImage(_T("test.jxr")));
    ASSERT_TRUE(TestMocks::IsImage(_T("test.heif")));
    ASSERT_TRUE(TestMocks::IsImage(_T("test.heic")));
}

END_TEST_SUITE()

///////////////////////////////////////////////
// Test Suite 6: Path Handling Tests
///////////////////////////////////////////////

TEST_SUITE(PathHandlingTests)

TEST_CASE(TestPathExtraction_WindowsPaths) {
    std::cout << "  Testing Windows path handling..." << std::endl;
    
    ASSERT_TRUE(TestMocks::IsImage(_T("C:\\Users\\Test\\image.jpg")));
    ASSERT_TRUE(TestMocks::IsImage(_T("C:\\Program Files\\app\\data\\photo.png")));
    ASSERT_EQUAL(TestMocks::GetLENSType(_T("D:\\Comics\\book.cbz")), LENSTYPE_CBZ);
}

TEST_CASE(TestPathExtraction_NetworkPaths) {
    std::cout << "  Testing UNC network path handling..." << std::endl;
    
    ASSERT_TRUE(TestMocks::IsImage(_T("\\\\server\\share\\image.webp")));
    ASSERT_EQUAL(TestMocks::GetLENSType(_T("\\\\nas\\comics\\book.cbr")), LENSTYPE_CBR);
}

TEST_CASE(TestPathExtraction_RelativePaths) {
    std::cout << "  Testing relative path handling..." << std::endl;
    
    ASSERT_TRUE(TestMocks::IsImage(_T("..\\folder\\image.avif")));
    ASSERT_TRUE(TestMocks::IsImage(_T(".\\current\\pic.jxl")));
    ASSERT_EQUAL(TestMocks::GetLENSType(_T("subfolder\\archive.7z")), LENSTYPE_7Z);
}

TEST_CASE(TestPathExtraction_LongPaths) {
    std::cout << "  Testing long path handling..." << std::endl;
    
    std::wstring longPath = L"C:\\";
    for (int i = 0; i < 50; i++) {
        longPath += L"verylongfoldername\\";
    }
    longPath += L"image.png";
    
    ASSERT_TRUE(TestMocks::IsImage(longPath.c_str()));
}

TEST_CASE(TestPathExtraction_SpecialCharacters) {
    std::cout << "  Testing paths with special characters..." << std::endl;
    
    ASSERT_TRUE(TestMocks::IsImage(_T("C:\\Users\\Test (Admin)\\My Photos\\pic.jpg")));
    ASSERT_TRUE(TestMocks::IsImage(_T("C:\\Files & Folders\\image_2024-11-18.webp")));
    ASSERT_EQUAL(TestMocks::GetLENSType(_T("C:\\Comics [Collection]\\book.cbz")), LENSTYPE_CBZ);
}

END_TEST_SUITE()

///////////////////////////////////////////////
// Test Suite 7: Boundary & Stress Tests
///////////////////////////////////////////////

TEST_SUITE(BoundaryTests)

TEST_CASE(TestEmptyStrings) {
    std::cout << "  Testing empty string handling..." << std::endl;
    
    ASSERT_FALSE(TestMocks::IsImage(_T("")));
    ASSERT_EQUAL(TestMocks::GetLENSType(_T("")), LENSTYPE_NONE);
}

TEST_CASE(TestNullTerminatedStrings) {
    std::cout << "  Testing null-terminated strings..." << std::endl;
    
    WCHAR buffer[256];
    wcscpy_s(buffer, _T("test.jpg"));
    ASSERT_TRUE(TestMocks::IsImage(buffer));
    
    wcscpy_s(buffer, _T("archive.cbz"));
    ASSERT_EQUAL(TestMocks::GetLENSType(PathFindExtension(buffer)), LENSTYPE_CBZ);
}

TEST_CASE(TestVeryLongFilenames) {
    std::cout << "  Testing very long filenames..." << std::endl;
    
    std::wstring longName(500, L'a');
    longName += L".jpg";
    ASSERT_TRUE(TestMocks::IsImage(longName.c_str()));
    
    longName = std::wstring(500, L'b') + L".cbz";
    LPWSTR ext = PathFindExtension(longName.c_str());
    ASSERT_EQUAL(TestMocks::GetLENSType(ext), LENSTYPE_CBZ);
}

TEST_CASE(TestMultipleDotsInFilename) {
    std::cout << "  Testing multiple dots in filename..." << std::endl;
    
    ASSERT_TRUE(TestMocks::IsImage(_T("my.photo.backup.jpg")));
    ASSERT_TRUE(TestMocks::IsImage(_T("file.v1.2.3.png")));
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".backup.copy.cbz")), LENSTYPE_CBZ);
}

TEST_CASE(TestUnicodeFilenames) {
    std::cout << "  Testing Unicode filenames..." << std::endl;
    
    // Japanese characters
    ASSERT_TRUE(TestMocks::IsImage(_T("画像.jpg")));
    // Russian characters  
    ASSERT_TRUE(TestMocks::IsImage(_T("фото.png")));
    // Arabic characters
    ASSERT_TRUE(TestMocks::IsImage(_T("صورة.webp")));
    
    // Extensions should still work
    LPWSTR ext1 = PathFindExtension(_T("画像.jpg"));
    LPWSTR ext2 = PathFindExtension(_T("komiks.cbz"));
    ASSERT_TRUE(TestMocks::IsImage(_T("画像.jpg")));
    ASSERT_EQUAL(TestMocks::GetLENSType(ext2), LENSTYPE_CBZ);
}

END_TEST_SUITE()

///////////////////////////////////////////////
// Test Suite 8: Integration Validation Tests
///////////////////////////////////////////////

TEST_SUITE(IntegrationTests)

TEST_CASE(TestRealWorldComicBookNames) {
    std::cout << "  Testing real-world comic book filenames..." << std::endl;
    
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".cbz")), LENSTYPE_CBZ);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".cbr")), LENSTYPE_CBR);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".cb7")), LENSTYPE_CB7);
    
    // Common patterns
    std::wstring ext = PathFindExtension(L"Batman Vol 1 #001 (1940).cbz");
    ASSERT_EQUAL(TestMocks::GetLENSType(ext.c_str()), LENSTYPE_CBZ);
    
    ext = PathFindExtension(L"Spider-Man_Annual_001.cbr");
    ASSERT_EQUAL(TestMocks::GetLENSType(ext.c_str()), LENSTYPE_CBR);
}

TEST_CASE(TestRealWorldEbookNames) {
    std::cout << "  Testing real-world ebook filenames..." << std::endl;
    
    std::wstring ext = PathFindExtension(L"The Great Book.epub");
    ASSERT_EQUAL(TestMocks::GetLENSType(ext.c_str()), LENSTYPE_EPUB);
    
    ext = PathFindExtension(L"Novel_2024.mobi");
    ASSERT_EQUAL(TestMocks::GetLENSType(ext.c_str()), LENSTYPE_MOBI);
    
    ext = PathFindExtension(L"Kindle_Book.azw3");
    ASSERT_EQUAL(TestMocks::GetLENSType(ext.c_str()), LENSTYPE_MOBI);
}

TEST_CASE(TestRealWorldImageNames) {
    std::cout << "  Testing real-world image filenames..." << std::endl;
    
    ASSERT_TRUE(TestMocks::IsImage(_T("IMG_20241118_123456.jpg")));
    ASSERT_TRUE(TestMocks::IsImage(_T("screenshot-2024-11-18.png")));
    ASSERT_TRUE(TestMocks::IsImage(_T("photo (1 of 10).webp")));
    ASSERT_TRUE(TestMocks::IsImage(_T("cover-art-final-v2.avif")));
}

TEST_CASE(TestMixedCaseExtensions) {
    std::cout << "  Testing mixed case extensions (real world)..." << std::endl;
    
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".Cbz")), LENSTYPE_CBZ);
    ASSERT_EQUAL(TestMocks::GetLENSType(_T(".ePub")), LENSTYPE_EPUB);
    ASSERT_TRUE(TestMocks::IsImage(_T("file.JpG")));
    ASSERT_TRUE(TestMocks::IsImage(_T("file.WebP")));
}

END_TEST_SUITE()

///////////////////////////////////////////////
// Main Test Runner
///////////////////////////////////////////////

void PrintTestHeader() {
    std::cout << "=========================================" << std::endl;
    std::cout << "ExplorerLens Unit Test Suite" << std::endl;
    std::cout << "Version: 4.6 (Windows 11 Modernized)" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << std::endl;
}

void PrintTestSummary() {
    std::cout << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Test Summary" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Tests Passed: " << g_testsPassed << std::endl;
    std::cout << "Tests Failed: " << g_testsFailed << std::endl;
    std::cout << "Total Tests:  " << (g_testsPassed + g_testsFailed) << std::endl;
    
    if (g_testsFailed == 0) {
        std::cout << std::endl;
        std::cout << "*** ALL TESTS PASSED ***" << std::endl;
    } else {
        std::cout << std::endl;
        std::cout << "!!! SOME TESTS FAILED !!!" << std::endl;
    }
    std::cout << "=========================================" << std::endl;
}

int main() {
    PrintTestHeader();
    
    std::cout << "[Suite 1/8] Format Detection Tests" << std::endl;
    RUN_TEST(FormatDetectionTests::TestGetLENSType_ComicArchives);
    RUN_TEST(FormatDetectionTests::TestGetLENSType_GenericArchives);
    RUN_TEST(FormatDetectionTests::TestGetLENSType_EbookFormats);
    RUN_TEST(FormatDetectionTests::TestGetLENSType_PhotoArchives);
    RUN_TEST(FormatDetectionTests::TestGetLENSType_UnsupportedFormats);
    RUN_TEST(FormatDetectionTests::TestGetLENSType_EdgeCases);
    std::cout << std::endl;
    
    std::cout << "[Suite 2/8] Image Format Tests" << std::endl;
    RUN_TEST(ImageFormatTests::TestIsImage_TraditionalFormats);
    RUN_TEST(ImageFormatTests::TestIsImage_ModernFormats);
    RUN_TEST(ImageFormatTests::TestIsImage_CaseInsensitive);
    RUN_TEST(ImageFormatTests::TestIsImage_WithPaths);
    RUN_TEST(ImageFormatTests::TestIsImage_NonImageFiles);
    RUN_TEST(ImageFormatTests::TestIsImage_EdgeCases);
    std::cout << std::endl;
    
    std::cout << "[Suite 3/8] String Utility Tests" << std::endl;
    RUN_TEST(StringUtilityTests::TestStrEqual_BasicComparison);
    RUN_TEST(StringUtilityTests::TestStrEqual_CaseInsensitive);
    RUN_TEST(StringUtilityTests::TestStrEqual_SpecialCharacters);
    std::cout << std::endl;
    
    std::cout << "[Suite 4/8] Format Coverage Tests" << std::endl;
    RUN_TEST(FormatCoverageTests::TestAllArchiveFormatsSupported);
    RUN_TEST(FormatCoverageTests::TestAllImageFormatsSupported);
    RUN_TEST(FormatCoverageTests::TestNoFormatCollisions);
    std::cout << std::endl;
    
    std::cout << "[Suite 5/8] Regression Tests" << std::endl;
    RUN_TEST(RegressionTests::TestBackwardCompatibility_OriginalFormats);
    RUN_TEST(RegressionTests::TestBackwardCompatibility_OriginalImages);
    RUN_TEST(RegressionTests::TestEnhancement_NewArchiveFormats);
    RUN_TEST(RegressionTests::TestEnhancement_NewImageFormats);
    std::cout << std::endl;
    
    std::cout << "[Suite 6/8] Path Handling Tests" << std::endl;
    RUN_TEST(PathHandlingTests::TestPathExtraction_WindowsPaths);
    RUN_TEST(PathHandlingTests::TestPathExtraction_NetworkPaths);
    RUN_TEST(PathHandlingTests::TestPathExtraction_RelativePaths);
    RUN_TEST(PathHandlingTests::TestPathExtraction_LongPaths);
    RUN_TEST(PathHandlingTests::TestPathExtraction_SpecialCharacters);
    std::cout << std::endl;
    
    std::cout << "[Suite 7/8] Boundary & Stress Tests" << std::endl;
    RUN_TEST(BoundaryTests::TestEmptyStrings);
    RUN_TEST(BoundaryTests::TestNullTerminatedStrings);
    RUN_TEST(BoundaryTests::TestVeryLongFilenames);
    RUN_TEST(BoundaryTests::TestMultipleDotsInFilename);
    RUN_TEST(BoundaryTests::TestUnicodeFilenames);
    std::cout << std::endl;
    
    std::cout << "[Suite 8/8] Integration Validation Tests" << std::endl;
    RUN_TEST(IntegrationTests::TestRealWorldComicBookNames);
    RUN_TEST(IntegrationTests::TestRealWorldEbookNames);
    RUN_TEST(IntegrationTests::TestRealWorldImageNames);
    RUN_TEST(IntegrationTests::TestMixedCaseExtensions);
    std::cout << std::endl;
    
    PrintTestSummary();
    
    return (g_testsFailed == 0) ? 0 : 1;
}

