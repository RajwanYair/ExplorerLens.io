# ===========================================================================
# Find-MSBuild.ps1
# Locates MSBuild executable
# ===========================================================================

$ErrorActionPreference = "Stop"

# Try vswhere first
$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $VsWhere) {
    $VsPath = & $VsWhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    if ($VsPath) {
        $MSBuild = Join-Path $VsPath "MSBuild\Current\Bin\amd64\MSBuild.exe"
        if (Test-Path $MSBuild) {
            return $MSBuild
        }
    }
}

# Fallback: search common paths
$CommonPaths = @(
    "${env:ProgramFiles}\Microsoft Visual Studio\2026\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\2025\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
)

foreach ($Path in $CommonPaths) {
    if (Test-Path $Path) {
        return $Path
    }
}

Write-Host "ERROR: MSBuild not found" -ForegroundColor Red
Write-Host "Please install Visual Studio Build Tools" -ForegroundColor Yellow
return $null
