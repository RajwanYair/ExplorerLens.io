# Find-All-Tools.ps1
# Detects all required build tools for DarkThumbs

$ErrorActionPreference = "Continue"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Build Tools Detector" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$tools = @{
    "Visual Studio Path" = $null
    "vcvarsall.bat"      = $null
    "MSBuild.exe"        = $null
    "CMake"              = $null
    "Ninja"              = $null
    "vcpkg"              = $null
}

# Find Visual Studio
$vsPaths = @(
    "C:\Program Files (x86)\Microsoft Visual Studio\2026\BuildTools",
    "C:\Program Files (x86)\Microsoft Visual Studio\2026\Community",
    "C:\Program Files (x86)\Microsoft Visual Studio\2026\Professional",
    "C:\Program Files (x86)\Microsoft Visual Studio\2026\Enterprise",
    "C:\Program Files (x86)\Microsoft Visual Studio\2025\BuildTools",
    "C:\Program Files (x86)\Microsoft Visual Studio\2025\Community",
    "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools",
    "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community",
    "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools",
    "C:\Program Files (x86)\Microsoft Visual Studio\17\BuildTools",
    "C:\Program Files\Microsoft Visual Studio\2026\BuildTools",
    "C:\Program Files\Microsoft Visual Studio\2025\BuildTools",
    "C:\Program Files\Microsoft Visual Studio\2022\BuildTools"
)

foreach ($path in $vsPaths) {
    if (Test-Path $path) {
        $tools["Visual Studio Path"] = $path
        break
    }
}

# Find vcvarsall.bat
if ($tools["Visual Studio Path"]) {
    $vcvarsall = Join-Path $tools["Visual Studio Path"] "VC\Auxiliary\Build\vcvarsall.bat"
    if (Test-Path $vcvarsall) {
        $tools["vcvarsall.bat"] = $vcvarsall
    }
}

# Find MSBuild using vswhere
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $msbuildPath = & $vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe 2>$null | Select-Object -First 1
    if ($msbuildPath -and (Test-Path $msbuildPath)) {
        $tools["MSBuild.exe"] = $msbuildPath
    }
}

# Fallback MSBuild search
if (-not $tools["MSBuild.exe"]) {
    $msbuildPaths = @(
        "C:\Program Files (x86)\Microsoft Visual Studio\2026\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2026\Community\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2025\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
    )
    
    foreach ($path in $msbuildPaths) {
        if (Test-Path $path) {
            $tools["MSBuild.exe"] = $path
            break
        }
    }
}

# Find CMake
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmake) {
    $tools["CMake"] = $cmake.Source
} else {
    # Check common paths
    $cmakePaths = @(
        "$env:ProgramFiles\CMake\bin\cmake.exe",
        "$env:ProgramFiles(x86)\CMake\bin\cmake.exe",
        "$env:USERPROFILE\scoop\shims\cmake.exe",
        "$env:LOCALAPPDATA\Programs\CMake\bin\cmake.exe"
    )
    
    foreach ($path in $cmakePaths) {
        if (Test-Path $path) {
            $tools["CMake"] = $path
            break
        }
    }
}

# Find Ninja
$ninja = Get-Command ninja -ErrorAction SilentlyContinue
if ($ninja) {
    $tools["Ninja"] = $ninja.Source
} else {
    # Check common paths
    $ninjaPaths = @(
        "$env:USERPROFILE\scoop\shims\ninja.exe",
        "C:\Tools\ninja\ninja.exe",
        "$env:LOCALAPPDATA\Programs\ninja\ninja.exe",
        "$env:ProgramFiles\Ninja\ninja.exe"
    )
    
    foreach ($path in $ninjaPaths) {
        if (Test-Path $path) {
            $tools["Ninja"] = $path
            break
        }
    }
}

# Find vcpkg
$vcpkg = Get-Command vcpkg -ErrorAction SilentlyContinue
if ($vcpkg) {
    $tools["vcpkg"] = $vcpkg.Source
} else {
    # Check common paths
    $vcpkgPaths = @(
        "$env:USERPROFILE\scoop\apps\vcpkg\current\vcpkg.exe",
        "$env:USERPROFILE\vcpkg\vcpkg.exe",
        "C:\vcpkg\vcpkg.exe",
        "C:\Tools\vcpkg\vcpkg.exe",
        "$env:VCPKG_ROOT\vcpkg.exe"
    )
    
    foreach ($path in $vcpkgPaths) {
        if (Test-Path $path) {
            $tools["vcpkg"] = $path
            break
        }
    }
}

# Display results
Write-Host "Tool Detection Results:" -ForegroundColor White
Write-Host ""

$allFound = $true
foreach ($tool in $tools.Keys | Sort-Object) {
    if ($tools[$tool]) {
        Write-Host "  ✓ $($tool.PadRight(25))" -ForegroundColor Green -NoNewline
        Write-Host " $($tools[$tool])" -ForegroundColor Gray
    } else {
        Write-Host "  ✗ $($tool.PadRight(25)) NOT FOUND" -ForegroundColor Red
        $allFound = $false
    }
}

Write-Host ""

if ($allFound) {
    Write-Host "All build tools found!" -ForegroundColor Green
    Write-Host "You can now build DarkThumbs." -ForegroundColor Cyan
} else {
    Write-Host "Some tools are missing." -ForegroundColor Yellow
    Write-Host "Please refer to .github\WINDOWS_BUILD_TOOLS.md for installation instructions." -ForegroundColor Yellow
}

Write-Host ""

# Return tools object
return $tools
