# ===========================================================================
# Verify-All-Paths-V7.ps1
# Verify all scripts use correct paths for external libraries
# ===========================================================================

param(
    [switch]$AutoFix,
    [switch]$Verbose
)

$ErrorActionPreference = "Continue"

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "ExplorerLens Path Validation (V7)" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

$WorkspaceRoot = (Get-Location).Path
$ErrorCount = 0
$WarningCount = 0

# Define incorrect path patterns and their correct replacements
$PathPatterns = @(
    @{
        Pattern = 'external\\compression[^-]'
        Replacement = 'external\compression-libs\'
        Description = "Incorrect compression library path (should be compression-libs)"
        Severity = "Error"
    },
    @{
        Pattern = 'external\\image-libs\\libwebp-1\.5\.0(?!-build)'
        Replacement = 'external\image-libs\libwebp-1.5.0-build'
        Description = "Incorrect libwebp path (should be libwebp-1.5.0-build)"
        Severity = "Error"
    },
    @{
        Pattern = 'ROADMAP\.md'
        Replacement = 'MASTER_PLAN.md'
        Description = "Reference to deleted ROADMAP.md (should be MASTER_PLAN.md)"
        Severity = "Warning"
    }
)

# Files/directories to exclude from validation
$ExcludePatterns = @(
    '**\build\**',
    '**\x64\**',
    '**\.vs\**',
    '**\packages\**',
    '**\external\**',
    '**\node_modules\**',
    '**\archive\**',
    '**\legacy\**'
)

Write-Host "[1] Scanning workspace for incorrect paths..." -ForegroundColor Yellow

function Test-PathPattern {
    param(
        [string]$FilePath,
        [hashtable]$Pattern
    )
    
    $content = Get-Content $FilePath -Raw -ErrorAction SilentlyContinue
    if (-not $content) { return @() }
    
    $matches = [regex]::Matches($content, $Pattern.Pattern)
    if ($matches.Count -eq 0) { return @() }
    
    $results = @()
    foreach ($match in $matches) {
        # Get line number
        $lineNumber = ($content.Substring(0, $match.Index) -split "`n").Count
        
        $results += @{
            File = $FilePath
            Line = $lineNumber
            Match = $match.Value
            Pattern = $Pattern
        }
    }
    
    return $results
}

# Get all script files
$AllFiles = Get-ChildItem -Path $WorkspaceRoot -Include *.ps1,*.md,*.txt,*.iss -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { 
        $path = $_.FullName
        -not ($ExcludePatterns | Where-Object { $path -like $_ })
    }

Write-Host "Found $($AllFiles.Count) files to validate" -ForegroundColor Gray

$Issues = @()

foreach ($file in $AllFiles) {
    foreach ($pattern in $PathPatterns) {
        $results = Test-PathPattern -FilePath $file.FullName -Pattern $pattern
        if ($results.Count -gt 0) {
            $Issues += $results
        }
    }
}

# Display results
Write-Host ""
Write-Host "[2] Validation Results" -ForegroundColor Yellow
Write-Host ""

if ($Issues.Count -eq 0) {
    Write-Host "✅ All paths are correct!" -ForegroundColor Green
    exit 0
}

$GroupedIssues = $Issues | Group-Object { $_.Pattern.Description }

foreach ($group in $GroupedIssues) {
    $severity = $group.Group[0].Pattern.Severity
    $color = if ($severity -eq "Error") { "Red" } else { "Yellow" }
    
    Write-Host "[$severity] $($group.Name)" -ForegroundColor $color
    Write-Host "  Found $($group.Count) occurrence(s):" -ForegroundColor Gray
    
    $fileGroups = $group.Group | Group-Object File
    foreach ($fileGroup in $fileGroups) {
        $relativePath = $fileGroup.Name.Replace($WorkspaceRoot, "").TrimStart('\')
        Write-Host "    - $relativePath" -ForegroundColor Gray
        
        if ($Verbose) {
            foreach ($item in $fileGroup.Group) {
                Write-Host "      Line $($item.Line): $($item.Match)" -ForegroundColor DarkGray
            }
        }
    }
    
    if ($severity -eq "Error") {
        $ErrorCount += $group.Count
    } else {
        $WarningCount += $group.Count
    }
    
    Write-Host ""
}

# Summary
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  Errors:   $ErrorCount" -ForegroundColor $(if ($ErrorCount -eq 0) { "Green" } else { "Red" })
Write-Host "  Warnings: $WarningCount" -ForegroundColor $(if ($WarningCount -eq 0) { "Green" } else { "Yellow" })
Write-Host "==========================================================================" -ForegroundColor Cyan

if ($AutoFix) {
    Write-Host ""
    Write-Host "[3] Auto-Fix Mode (NOT IMPLEMENTED)" -ForegroundColor Yellow
    Write-Host "Auto-fix requires manual review. Please update files manually or use:" -ForegroundColor Gray
    Write-Host "  build-scripts/utilities/Update-Library-Paths.ps1" -ForegroundColor Gray
}

# Exit with error code if errors found
exit $(if ($ErrorCount -gt 0) { 1 } else { 0 })

