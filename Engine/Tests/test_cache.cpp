// test_cache.cpp - Unit tests for ThumbnailCache
#include "../Cache/ThumbnailCache.h"
#include "../Core/Types.h"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>

using namespace ExplorerLens;
using namespace ExplorerLens::Engine;

// Test helpers
void PrintTestHeader(const char* testName) {
 std::wcout << L"Running: " << testName << L"..." << std::endl;
}

void PrintTestResult(bool passed) {
 if (passed) {
 std::wcout << L" [PASS]" << std::endl;
 } else {
 std::wcout << L" [FAIL]" << std::endl;
 }
}

// Create simple test bitmap
HBITMAP CreateTestBitmap(int width, int height, COLORREF color) {
 HDC hdc = GetDC(nullptr);
 HDC memDC = CreateCompatibleDC(hdc);
 HBITMAP hBitmap = CreateCompatibleBitmap(hdc, width, height);
 
 HGDIOBJ oldBitmap = SelectObject(memDC, hBitmap);
 
 // Fill with color
 RECT rect = { 0, 0, width, height };
 HBRUSH hBrush = CreateSolidBrush(color);
 FillRect(memDC, &rect, hBrush);
 DeleteObject(hBrush);
 
 SelectObject(memDC, oldBitmap);
 DeleteDC(memDC);
 ReleaseDC(nullptr, hdc);
 
 return hBitmap;
}

// Test 1: Create cache
bool TestCache_Create() {
 PrintTestHeader("TestCache_Create");
 
 ICacheProvider* pCache = CreateThumbnailCache();
 bool passed = (pCache != nullptr);
 
 if (pCache) {
 delete pCache;
 }
 
 PrintTestResult(passed);
 return passed;
}

// Test 2: Initialize cache
bool TestCache_Initialize() {
 PrintTestHeader("TestCache_Initialize");
 
 ThumbnailCache* pCache = static_cast<ThumbnailCache*>(CreateThumbnailCache());
 if (!pCache) {
 PrintTestResult(false);
 return false;
 }
 
 HRESULT hr = pCache->Initialize(100); // 100 MB max
 bool passed = SUCCEEDED(hr);
 
 delete pCache;
 PrintTestResult(passed);
 return passed;
}

// Test 3: Put and Get
bool TestCache_PutGet() {
 PrintTestHeader("TestCache_PutGet");
 
 ThumbnailCache* pCache = static_cast<ThumbnailCache*>(CreateThumbnailCache());
 if (!pCache) {
 PrintTestResult(false);
 return false;
 }
 
 HRESULT hr = pCache->Initialize(100);
 if (FAILED(hr)) {
 delete pCache;
 PrintTestResult(false);
 return false;
 }
 
 // Create test bitmap
 HBITMAP hTestBitmap = CreateTestBitmap(64, 64, RGB(255, 0, 0));
 if (!hTestBitmap) {
 delete pCache;
 PrintTestResult(false);
 return false;
 }
 
 // Put in cache
 const wchar_t* testPath = L"C:\\test\\image.jpg";
 hr = pCache->Put(testPath, 64, 64, hTestBitmap);
 DeleteObject(hTestBitmap);
 
 bool passed = SUCCEEDED(hr);
 
 if (passed) {
 // Get from cache
 HBITMAP hCachedBitmap = nullptr;
 hr = pCache->Get(testPath, 64, 64, &hCachedBitmap);
 passed = SUCCEEDED(hr) && (hCachedBitmap != nullptr);
 
 if (hCachedBitmap) {
 // Verify dimensions
 BITMAP bm;
 if (GetObject(hCachedBitmap, sizeof(BITMAP), &bm)) {
 passed = (bm.bmWidth == 64 && bm.bmHeight == 64);
 }
 DeleteObject(hCachedBitmap);
 }
 }
 
 delete pCache;
 PrintTestResult(passed);
 return passed;
}

// Test 4: Exists check
bool TestCache_Exists() {
 PrintTestHeader("TestCache_Exists");
 
 ThumbnailCache* pCache = static_cast<ThumbnailCache*>(CreateThumbnailCache());
 if (!pCache) {
 PrintTestResult(false);
 return false;
 }
 
 pCache->Initialize(100);
 
 const wchar_t* testPath = L"C:\\test\\exists.jpg";
 
 // Should not exist initially
 bool exists = pCache->Exists(testPath, 32, 32);
 bool passed = !exists;
 
 if (passed) {
 // Add to cache
 HBITMAP hBitmap = CreateTestBitmap(32, 32, RGB(0, 255, 0));
 pCache->Put(testPath, 32, 32, hBitmap);
 DeleteObject(hBitmap);
 
 // Should exist now
 exists = pCache->Exists(testPath, 32, 32);
 passed = exists;
 }
 
 delete pCache;
 PrintTestResult(passed);
 return passed;
}

// Test 5: Cache stats
bool TestCache_GetStats() {
 PrintTestHeader("TestCache_GetStats");
 
 ThumbnailCache* pCache = static_cast<ThumbnailCache*>(CreateThumbnailCache());
 if (!pCache) {
 PrintTestResult(false);
 return false;
 }
 
 pCache->Initialize(100);
 
 // Add entries
 for (int i = 0; i < 5; i++) {
 wchar_t path[256];
 swprintf_s(path, L"C:\\test\\file%d.jpg", i);
 
 HBITMAP hBitmap = CreateTestBitmap(32, 32, RGB(i * 50, 0, 0));
 pCache->Put(path, 32, 32, hBitmap);
 DeleteObject(hBitmap);
 }
 
 // Check stats
 uint32_t entryCount = 0;
 uint32_t totalSizeMB = 0;
 HRESULT hr = pCache->GetStats(&entryCount, &totalSizeMB);
 
 bool passed = SUCCEEDED(hr) && (entryCount == 5);
 
 if (passed) {
 std::wcout << L" Entries: " << entryCount << L", Size: " << totalSizeMB << L" MB" << std::endl;
 }
 
 delete pCache;
 PrintTestResult(passed);
 return passed;
}

// Test 6: Clear cache
bool TestCache_Clear() {
 PrintTestHeader("TestCache_Clear");
 
 ThumbnailCache* pCache = static_cast<ThumbnailCache*>(CreateThumbnailCache());
 if (!pCache) {
 PrintTestResult(false);
 return false;
 }
 
 pCache->Initialize(100);
 
 // Add entries
 for (int i = 0; i < 3; i++) {
 wchar_t path[256];
 swprintf_s(path, L"C:\\test\\clear%d.jpg", i);
 
 HBITMAP hBitmap = CreateTestBitmap(32, 32, RGB(0, 0, 255));
 pCache->Put(path, 32, 32, hBitmap);
 DeleteObject(hBitmap);
 }
 
 // Clear cache
 HRESULT hr = pCache->Clear();
 bool passed = SUCCEEDED(hr);
 
 if (passed) {
 // Verify empty
 uint32_t entryCount = 0;
 pCache->GetStats(&entryCount, nullptr);
 passed = (entryCount == 0);
 }
 
 delete pCache;
 PrintTestResult(passed);
 return passed;
}

// Test 7: Multiple sizes for same file
bool TestCache_MultipleSizes() {
 PrintTestHeader("TestCache_MultipleSizes");
 
 ThumbnailCache* pCache = static_cast<ThumbnailCache*>(CreateThumbnailCache());
 if (!pCache) {
 PrintTestResult(false);
 return false;
 }
 
 pCache->Initialize(100);
 
 const wchar_t* testPath = L"C:\\test\\multisize.jpg";
 uint32_t sizes[] = { 32, 64, 128, 256 };
 
 // Cache different sizes
 for (uint32_t size : sizes) {
 HBITMAP hBitmap = CreateTestBitmap(size, size, RGB(255, 255, 0));
 HRESULT hr = pCache->Put(testPath, size, size, hBitmap);
 DeleteObject(hBitmap);
 
 if (FAILED(hr)) {
 delete pCache;
 PrintTestResult(false);
 return false;
 }
 }
 
 // Verify all sizes exist
 bool passed = true;
 for (uint32_t size : sizes) {
 if (!pCache->Exists(testPath, size, size)) {
 passed = false;
 break;
 }
 
 // Verify retrieval
 HBITMAP hBitmap = nullptr;
 HRESULT hr = pCache->Get(testPath, size, size, &hBitmap);
 if (FAILED(hr) || !hBitmap) {
 passed = false;
 break;
 }
 
 // Check dimensions
 BITMAP bm;
 if (GetObject(hBitmap, sizeof(BITMAP), &bm)) {
 if (bm.bmWidth != (LONG)size || bm.bmHeight != (LONG)size) {
 passed = false;
 }
 }
 DeleteObject(hBitmap);
 }
 
 delete pCache;
 PrintTestResult(passed);
 return passed;
}

// Test 8: Cache miss
bool TestCache_Miss() {
 PrintTestHeader("TestCache_Miss");
 
 ThumbnailCache* pCache = static_cast<ThumbnailCache*>(CreateThumbnailCache());
 if (!pCache) {
 PrintTestResult(false);
 return false;
 }
 
 pCache->Initialize(100);
 
 // Try to get non-existent entry
 HBITMAP hBitmap = nullptr;
 HRESULT hr = pCache->Get(L"C:\\nonexistent.jpg", 64, 64, &hBitmap);
 
 bool passed = FAILED(hr) && (hBitmap == nullptr);
 
 delete pCache;
 PrintTestResult(passed);
 return passed;
}

// Test 9: Thread safety (basic)
bool TestCache_ThreadSafety() {
 PrintTestHeader("TestCache_ThreadSafety");
 
 ThumbnailCache* pCache = static_cast<ThumbnailCache*>(CreateThumbnailCache());
 if (!pCache) {
 PrintTestResult(false);
 return false;
 }
 
 pCache->Initialize(100);
 
 // Multiple threads accessing cache
 const int numThreads = 4;
 std::vector<std::thread> threads;
 bool success = true;
 
 for (int t = 0; t < numThreads; t++) {
 threads.emplace_back([pCache, t, &success]() {
 for (int i = 0; i < 10; i++) {
 wchar_t path[256];
 swprintf_s(path, L"C:\\test\\thread%d_%d.jpg", t, i);
 
 HBITMAP hBitmap = CreateTestBitmap(32, 32, RGB(t * 60, i * 25, 0));
 HRESULT hr = pCache->Put(path, 32, 32, hBitmap);
 DeleteObject(hBitmap);
 
 if (FAILED(hr)) {
 success = false;
 }
 
 // Small delay
 std::this_thread::sleep_for(std::chrono::milliseconds(1));
 }
 });
 }
 
 // Wait for all threads
 for (auto& thread : threads) {
 thread.join();
 }
 
 bool passed = success;
 
 delete pCache;
 PrintTestResult(passed);
 return passed;
}

// Test runner
void RunCacheTests() {
 std::wcout << L"\nCache Provider Tests:" << std::endl;
 
 int passed = 0;
 int total = 0;
 
 #define RUN_TEST(test) { total++; if (test()) passed++; }
 
 RUN_TEST(TestCache_Create);
 RUN_TEST(TestCache_Initialize);
 RUN_TEST(TestCache_PutGet);
 RUN_TEST(TestCache_Exists);
 RUN_TEST(TestCache_GetStats);
 RUN_TEST(TestCache_Clear);
 RUN_TEST(TestCache_MultipleSizes);
 RUN_TEST(TestCache_Miss);
 RUN_TEST(TestCache_ThreadSafety);
 
 #undef RUN_TEST
 
 std::wcout << L" Cache Tests: " << passed << L"/" << total << L" passed" << std::endl;
}

