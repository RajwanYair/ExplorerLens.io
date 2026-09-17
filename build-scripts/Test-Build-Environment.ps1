#Requires -Version 7.0

<#
.SYNOPSIS
    Test and validate build environment for ExplorerLens
.DESCRIPTION
    Checks all prerequisites and external library directories
#>

$ErrorActionPreference = "Continue"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  ExplorerLens Build Environment Test" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$rootDir = Split-Path -Parent $PSScriptRoot

$results = @()
$machinePathEntries = @(
    [Environment]::GetEnvironmentVariable('Path', 'Machine') -split ';' |
        Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
        ForEach-Object { $_.TrimEnd('\') }
)
$machineToolCandidates = @{
    cmake   = @('C:\Program Files\CMake\bin\cmake.exe', 'C:\ProgramData\scoop\shims\cmake.exe')
    ninja   = @('C:\Program Files\Ninja\ninja.exe', 'C:\ProgramData\scoop\shims\ninja.exe')
    meson   = @('C:\Program Files\Meson\meson.exe', 'C:\ProgramData\scoop\shims\meson.exe')
    nasm    = @('C:\Program Files\NASM\nasm.exe', 'C:\ProgramData\scoop\shims\nasm.exe')
    sccache = @('C:\Program Files\sccache\sccache.exe', 'C:\ProgramData\scoop\shims\sccache.exe')
    nuget   = @('C:\Program Files\NuGet\nuget.exe', 'C:\ProgramData\scoop\shims\nuget.exe')
    wix     = @('C:\Program Files\WiX Toolset v6.0\bin\wix.exe', 'C:\ProgramData\dotnet-tools\wix.exe', 'C:\ProgramData\scoop\shims\wix.exe')
    git     = @('C:\Program Files\Git\cmd\git.exe')
    dotnet  = @('C:\Program Files\dotnet\dotnet.exe', 'C:\ProgramData\scoop\apps\dotnet-sdk\current\dotnet.exe')
}

function Resolve-MachineTool {
    param([Parameter(Mandatory)][string]$Name)

    $candidates = @(
        $machinePathEntries | ForEach-Object { Join-Path $_ "$Name.exe" }
        (Join-Path 'C:\ProgramData\scoop\apps' "$Name\current\$Name.exe")
        $machineToolCandidates[$Name]
    ) | Where-Object { $_ } | Select-Object -Unique

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return [PSCustomObject]@{ Path = $candidate; Scope = 'Machine' }
        }
    }

    $userCommand = Get-Command "$Name.exe" -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($userCommand) {
        return [PSCustomObject]@{ Path = $userCommand.Source; Scope = 'User' }
    }

    return [PSCustomObject]@{ Path = $null; Scope = 'Missing' }
}

# Test Visual Studio Build Tools
Write-Host "[1] Visual Studio Build Tools" -ForegroundColor Yellow
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath = if (Test-Path -LiteralPath $vswhere) {
    & $vswhere -latest -products '*' -version '[18.0,19.0)' `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
}
$vcvarsPath = if ($vsPath) { Join-Path $vsPath 'VC\Auxiliary\Build\vcvars64.bat' } else { 'N/A' }
if (Test-Path -LiteralPath $vcvarsPath) {
    Write-Host "  ✅ vcvars64.bat found" -ForegroundColor Green
    $results += [PSCustomObject]@{ Component = "VS Build Tools"; Status = "✅ OK"; Path = $vcvarsPath }
} else {
    Write-Host "  ❌ vcvars64.bat NOT found" -ForegroundColor Red
    $results += [PSCustomObject]@{ Component = "VS Build Tools"; Status = "❌ MISSING"; Path = $vcvarsPath }
}

$toolset = if ($vsPath) {
    Get-ChildItem -LiteralPath (Join-Path $vsPath 'VC\Tools\MSVC') -Directory -ErrorAction SilentlyContinue |
        Where-Object Name -Match '^14\.5\d\.' |
        Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
}
foreach ($payload in @('bin\Hostx64\x64\cl.exe', 'bin\Hostx64\x64\link.exe', 'atlmfc\include\atlbase.h')) {
    $path = if ($toolset) { Join-Path $toolset.FullName $payload } else { 'N/A' }
    $status = if (Test-Path -LiteralPath $path -PathType Leaf) { 'FOUND' } else { 'MISSING' }
    $results += [PSCustomObject]@{ Component = "MSVC v145 $payload"; Status = $status; Path = $path }
}
$sdkRoot = "${env:ProgramFiles(x86)}\Windows Kits\10"
foreach ($payload in @('Include\10.0.26100.0\um\Windows.h', 'Lib\10.0.26100.0\um\x64\kernel32.lib', 'bin\10.0.26100.0\x64\rc.exe', 'bin\10.0.26100.0\x64\mt.exe')) {
    $path = Join-Path $sdkRoot $payload
    $status = if (Test-Path -LiteralPath $path -PathType Leaf) { 'FOUND' } else { 'MISSING' }
    $results += [PSCustomObject]@{ Component = "Windows SDK $payload"; Status = $status; Path = $path }
}

# Test build tools
Write-Host "`n[2] CMake" -ForegroundColor Yellow
$cmake = Resolve-MachineTool 'cmake'
if ($cmake.Path) {
    $versionOutput = & $cmake.Path --version
    $version = $versionOutput | Select-Object -First 1
    $status = if ($LASTEXITCODE -eq 0 -and $version -match '^cmake version (\d+\.\d+\.\d+)' -and [version]$Matches[1] -ge [version]'4.2.0') {
        if ($cmake.Scope -eq 'Machine') { 'OK' } else { 'NONCOMPLIANT (USER-SCOPED)' }
    } else { 'OUTDATED (requires CMake 4.2+)' }
    Write-Host "  $version - $status"
    $results += [PSCustomObject]@{ Component = 'CMake'; Status = $status; Path = $cmake.Path }
} else {
    Write-Host "  ❌ CMake NOT found in machine PATH or machine-wide roots" -ForegroundColor Red
    $results += [PSCustomObject]@{ Component = "CMake"; Status = "❌ MISSING"; Path = "N/A" }
}

foreach ($tool in @('ninja', 'git', 'meson', 'nasm', 'nuget', 'dotnet', 'wix')) {
    $command = Resolve-MachineTool $tool
    if ($command.Path) {
        $status = if ($command.Scope -eq 'Machine') { 'OK' } else { 'NONCOMPLIANT (USER-SCOPED)' }
        $results += [PSCustomObject]@{ Component = $tool; Status = $status; Path = $command.Path }
    } else {
        $results += [PSCustomObject]@{ Component = $tool; Status = 'MISSING'; Path = 'N/A' }
    }
}

# Test external libraries
Write-Host "`n[3] External Libraries" -ForegroundColor Yellow

$libraries = @{
    "zlib-1.3.1"         = "external\compression-libs\zlib-1.3.1"
    "lz4-1.10.0"         = "external\compression-libs\lz4-1.10.0"
    "zstd-1.5.7"         = "external\compression-libs\zstd-1.5.7"
    "xz-5.6.3 (liblzma)" = "external\compression-libs\xz-5.6.3"
    "minizip-ng-4.0.10"  = "external\compression-libs\minizip-ng-4.0.10"
    "libwebp-1.5.0"      = "external\image-libs\libwebp-1.5.0-build"
    "dav1d-1.5.1"        = "external\image-libs\dav1d-1.5.1"
    "libavif-1.3.0"      = "external\image-libs\libavif-1.3.0"
    "libjxl-0.11.1"      = "external\image-libs\libjxl-0.11.1"
}

foreach ($lib in $libraries.GetEnumerator()) {
    $path = Join-Path $rootDir $lib.Value
    if (Test-Path $path) {
        Write-Host "  ✅ $($lib.Key)" -ForegroundColor Green
        $results += [PSCustomObject]@{ Component = $lib.Key; Status = "✅ FOUND"; Path = $lib.Value }
    } else {
        Write-Host "  ❌ $($lib.Key) - NOT FOUND" -ForegroundColor Red
        $results += [PSCustomObject]@{ Component = $lib.Key; Status = "❌ MISSING"; Path = $lib.Value }
    }
}

# Test critical build files
Write-Host "`n[4] Critical Build Files" -ForegroundColor Yellow

$criticalFiles = @{
    "libwebp Makefile.vc"       = "external\image-libs\libwebp-1.5.0-build\Makefile.vc"
    "minizip-ng CMakeLists.txt" = "external\compression-libs\minizip-ng-4.0.10\CMakeLists.txt"
}

foreach ($file in $criticalFiles.GetEnumerator()) {
    $path = Join-Path $rootDir $file.Value
    if (Test-Path $path) {
        Write-Host "  ✅ $($file.Key)" -ForegroundColor Green
        $results += [PSCustomObject]@{ Component = $file.Key; Status = 'FOUND'; Path = $file.Value }
    } else {
        Write-Host "  ❌ $($file.Key) - NOT FOUND" -ForegroundColor Red
        $results += [PSCustomObject]@{ Component = $file.Key; Status = 'MISSING'; Path = $file.Value }
    }
}

# Summary
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Summary" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$passed = ($results | Where-Object { $_.Status -eq 'OK' -or $_.Status -like "*FOUND*" }).Count
$failed = ($results | Where-Object { $_.Status -match 'MISSING|NONCOMPLIANT|OUTDATED' }).Count

$results | Format-Table -AutoSize

Write-Host "Passed: $passed" -ForegroundColor Green
Write-Host "Failed: $failed" -ForegroundColor $(if ($failed -gt 0) { "Red" } else { "Green" })

if ($failed -eq 0) {
    Write-Host "`nPrerequisite checks passed. Compiled libraries, linking, and runtime behavior are not verified." -ForegroundColor Green
    exit 0
} else {
    Write-Host "`n❌ Build environment has missing components!" -ForegroundColor Red
    exit 1
}
