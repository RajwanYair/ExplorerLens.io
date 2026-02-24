# ExplorerLens Test Suite
# PowerShell script for comprehensive testing and validation

param(
    [switch]$Quick,
    [switch]$Performance,
    [switch]$Compatibility,
    [string]$TestFilesPath = "$PSScriptRoot\tests\test-files"
)

$ErrorActionPreference = "Continue"

# Colors
function Write-Pass { param($msg) Write-Host "[OK] $msg" -ForegroundColor Green }
function Write-Fail { param($msg) Write-Host "[FAIL] $msg" -ForegroundColor Red }
function Write-Info { param($msg) Write-Host "[INFO] $msg" -ForegroundColor Cyan }
function Write-Warn { param($msg) Write-Host "[WARN] $msg" -ForegroundColor Yellow }

# Test results tracking
$script:testResults = @{
    Total = 0
    Passed = 0
    Failed = 0
    Skipped = 0
    Tests = @()
}

function Add-TestResult {
    param(
        [string]$Category,
        [string]$Test,
        [string]$Status,
        [string]$Message = ""
    )
    
    $script:testResults.Total++
    
    switch ($Status) {
        "Pass" { $script:testResults.Passed++; Write-Pass "$Category - $Test" }
        "Fail" { $script:testResults.Failed++; Write-Fail "$Category - $Test" }
        "Skip" { $script:testResults.Skipped++; Write-Warn "$Category - $Test (Skipped)" }
    }
    
    if ($Message) {
        Write-Info "  $Message"
    }
    
    $script:testResults.Tests += @{
        Category = $Category
        Test = $Test
        Status = $Status
        Message = $Message
        Timestamp = Get-Date
    }
}

# Banner
Write-Host ""
Write-Host "+========================================+" -ForegroundColor Cyan
Write-Host "|    ExplorerLens v5.2.0+ Test Suite       |" -ForegroundColor Cyan
Write-Host "+========================================+" -ForegroundColor Cyan
Write-Host ""

# Test 1: Installation verification
Write-Info "Test Category: Installation"
Write-Info "=============================="

$systemDll = "$env:SystemRoot\System32\LENSShell.dll"
if (Test-Path $systemDll) {
    Add-TestResult "Installation" "LENSShell.dll exists" "Pass"
    
    $dllInfo = Get-Item $systemDll
    $expectedSize = 1400000  # ~1.4 MB minimum
    
    if ($dllInfo.Length -gt $expectedSize) {
        Add-TestResult "Installation" "DLL size valid" "Pass" "$([math]::Round($dllInfo.Length/1MB, 2)) MB"
    } else {
        Add-TestResult "Installation" "DLL size valid" "Fail" "Size too small: $($dllInfo.Length) bytes"
    }
} else {
    Add-TestResult "Installation" "LENSShell.dll exists" "Fail" "DLL not found"
}

# Test 2: COM Registration
Write-Info ""
Write-Info "Test Category: COM Registration"
Write-Info "================================"

$clsid = "{3CA50AA2-92F6-4FD6-82C2-37E0AF7F967C}"
$regPath = "Registry::HKEY_CLASSES_ROOT\CLSID\$clsid"

if (Test-Path $regPath) {
    Add-TestResult "COM" "CLSID registered" "Pass"
    
    $inprocPath = "$regPath\InprocServer32"
    if (Test-Path $inprocPath) {
        Add-TestResult "COM" "InprocServer32 key exists" "Pass"
        
        $server = Get-ItemProperty -Path $inprocPath -ErrorAction SilentlyContinue
        if ($server.'(default)' -eq $systemDll) {
            Add-TestResult "COM" "DLL path correct" "Pass"
        } else {
            Add-TestResult "COM" "DLL path correct" "Fail" "Path mismatch"
        }
    } else {
        Add-TestResult "COM" "InprocServer32 key exists" "Fail"
    }
    
    # Check threading model
    $threadingModel = (Get-ItemProperty -Path $inprocPath -Name "ThreadingModel" -ErrorAction SilentlyContinue).ThreadingModel
    if ($threadingModel -eq "Apartment") {
        Add-TestResult "COM" "Threading model" "Pass" "Apartment"
    } else {
        Add-TestResult "COM" "Threading model" "Warn" "Not Apartment: $threadingModel"
    }
} else {
    Add-TestResult "COM" "CLSID registered" "Fail" "Registry key not found"
}

# Test 3: System Compatibility
Write-Info ""
Write-Info "Test Category: System Compatibility"
Write-Info "===================================="

$os = Get-CimInstance Win32_OperatingSystem
$build = [int]$os.BuildNumber

if ($build -ge 17134) {  # Windows 10 1803
    Add-TestResult "Compatibility" "Windows version" "Pass" "Build $build"
} else {
    Add-TestResult "Compatibility" "Windows version" "Fail" "Build $build (requires 17134+)"
}

if ([Environment]::Is64BitOperatingSystem) {
    Add-TestResult "Compatibility" "64-bit OS" "Pass"
} else {
    Add-TestResult "Compatibility" "64-bit OS" "Fail"
}

# Check DirectX
$dxdiag = Get-Command dxdiag.exe -ErrorAction SilentlyContinue
if ($dxdiag) {
    Add-TestResult "Compatibility" "DirectX available" "Pass"
} else {
    Add-TestResult "Compatibility" "DirectX available" "Warn"
}

# Test 4: Dependencies
Write-Info ""
Write-Info "Test Category: Dependencies"
Write-Info "==========================="

if (Test-Path $systemDll) {
    # Check for missing DLL dependencies using dumpbin if available
    $dumpbin = Get-Command dumpbin.exe -ErrorAction SilentlyContinue
    if ($dumpbin) {
        $deps = & dumpbin /dependents $systemDll 2>&1
        $systemDeps = $deps | Select-String -Pattern "dll" | Where-Object { $_ -notmatch "system32" }
        
        if ($systemDeps.Count -eq 0) {
            Add-TestResult "Dependencies" "No external dependencies" "Pass"
        } else {
            Add-TestResult "Dependencies" "No external dependencies" "Warn" "Found: $($systemDeps.Count)"
        }
    } else {
        Add-TestResult "Dependencies" "Dependency check" "Skip" "dumpbin not available"
    }
}

# Test 5: Performance
if ($Performance -or -not $Quick) {
    Write-Info ""
    Write-Info "Test Category: Performance"
    Write-Info "=========================="
    
    # Check available memory
    $mem = Get-CimInstance Win32_OperatingSystem
    $freeMemMB = [math]::Round($mem.FreePhysicalMemory / 1KB, 0)
    
    if ($freeMemMB -gt 1000) {
        Add-TestResult "Performance" "Available memory" "Pass" "$freeMemMB MB free"
    } else {
        Add-TestResult "Performance" "Available memory" "Warn" "Only $freeMemMB MB free"
    }
    
    # Check CPU
    $cpu = Get-CimInstance Win32_Processor | Select-Object -First 1
    $cores = $cpu.NumberOfLogicalProcessors
    
    if ($cores -ge 4) {
        Add-TestResult "Performance" "CPU cores" "Pass" "$cores logical processors"
    } else {
        Add-TestResult "Performance" "CPU cores" "Warn" "Only $cores logical processors"
    }
    
    # Check for AVX2 support (indirect check)
    $cpuFeatures = $cpu.Description
    if ($cpuFeatures -match "Intel|AMD") {
        Add-TestResult "Performance" "Modern CPU" "Pass" $cpu.Name
    }
}

# Test 6: File Associations (Thumbnail Handlers)
Write-Info ""
Write-Info "Test Category: File Associations"
Write-Info "================================"

$formats = @(
    @{Ext=".webp"; Name="WebP"},
    @{Ext=".heic"; Name="HEIF"},
    @{Ext=".avif"; Name="AVIF"},
    @{Ext=".pdf"; Name="PDF"},
    @{Ext=".rar"; Name="RAR"},
    @{Ext=".zip"; Name="ZIP"}
)

foreach ($format in $formats) {
    $regKey = "Registry::HKEY_CLASSES_ROOT\$($format.Ext)"
    if (Test-Path $regKey) {
        Add-TestResult "FileAssoc" "$($format.Name) extension registered" "Pass"
    } else {
        Add-TestResult "FileAssoc" "$($format.Name) extension registered" "Warn" "Extension not registered in HKCR"
    }
}

# Test 7: Thumbnail Cache
Write-Info ""
Write-Info "Test Category: Thumbnail Cache"
Write-Info "=============================="

$cachePath = "$env:LOCALAPPDATA\Microsoft\Windows\Explorer"
if (Test-Path $cachePath) {
    Add-TestResult "Cache" "Cache directory exists" "Pass"
    
    $cacheFiles = Get-ChildItem -Path $cachePath -Filter "thumbcache_*.db" -ErrorAction SilentlyContinue
    if ($cacheFiles.Count -gt 0) {
        Add-TestResult "Cache" "Cache files present" "Pass" "$($cacheFiles.Count) cache file(s)"
        
        $totalSize = ($cacheFiles | Measure-Object -Property Length -Sum).Sum
        $sizeMB = [math]::Round($totalSize / 1MB, 2)
        Add-TestResult "Cache" "Cache size" "Pass" "$sizeMB MB"
    } else {
        Add-TestResult "Cache" "Cache files present" "Warn" "No cache files found"
    }
} else {
    Add-TestResult "Cache" "Cache directory exists" "Fail"
}

# Test 8: Windows Explorer Integration
Write-Info ""
Write-Info "Test Category: Explorer Integration"
Write-Info "===================================="

$explorerProc = Get-Process explorer -ErrorAction SilentlyContinue
if ($explorerProc) {
    Add-TestResult "Explorer" "Explorer.exe running" "Pass"
    
    # Check if DLL is loaded (requires admin)
    if (([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        $modules = $explorerProc | ForEach-Object { $_.Modules } | Where-Object { $_.FileName -like "*LENSShell.dll" }
        if ($modules) {
            Add-TestResult "Explorer" "DLL loaded in Explorer" "Pass"
        } else {
            Add-TestResult "Explorer" "DLL loaded in Explorer" "Warn" "Not currently loaded (may load on demand)"
        }
    } else {
        Add-TestResult "Explorer" "DLL load check" "Skip" "Requires admin rights"
    }
} else {
    Add-TestResult "Explorer" "Explorer.exe running" "Fail"
}

# Test Summary
Write-Info ""
Write-Info "+========================================+" -ForegroundColor Cyan
Write-Info "|           Test Summary                 |" -ForegroundColor Cyan
Write-Info "+========================================+" -ForegroundColor Cyan
Write-Info ""
Write-Info "Total Tests: $($script:testResults.Total)"
Write-Pass "Passed: $($script:testResults.Passed)"
if ($script:testResults.Failed -gt 0) {
    Write-Fail "Failed: $($script:testResults.Failed)"
}
if ($script:testResults.Skipped -gt 0) {
    Write-Warn "Skipped: $($script:testResults.Skipped)"
}

$successRate = if ($script:testResults.Total -gt 0) {
    [math]::Round(($script:testResults.Passed / $script:testResults.Total) * 100, 1)
} else { 0 }

Write-Info "Success Rate: $successRate%"
Write-Info ""

# Overall verdict
if ($script:testResults.Failed -eq 0 -and $script:testResults.Passed -gt 0) {
    Write-Pass "OVERALL STATUS: ALL TESTS PASSED [OK]"
    exit 0
} elseif ($script:testResults.Failed -gt 0) {
    Write-Fail "OVERALL STATUS: SOME TESTS FAILED"
    exit 1
} else {
    Write-Warn "OVERALL STATUS: INCONCLUSIVE"
    exit 2
}

