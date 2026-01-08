# Remove Win32/x86 configurations from Visual Studio project files
# This script ensures the project only builds for x64 platform

param(
    [string]$ProjectPath = "c:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs"
)

$ErrorActionPreference = "Stop"

Write-Host "Removing Win32/x86 configurations from Visual Studio projects..." -ForegroundColor Cyan
Write-Host ""

$projectFiles = @(
    "CBXShell\CBXShell.vcxproj",
    "CBXManager\CBXManager.vcxproj"
)

foreach ($projFile in $projectFiles) {
    $fullPath = Join-Path $ProjectPath $projFile
    
    if (-not (Test-Path $fullPath)) {
        Write-Host "[SKIP] $projFile not found" -ForegroundColor Yellow
        continue
    }
    
    Write-Host "Processing: $projFile" -ForegroundColor Cyan
    
    # Load XML
    [xml]$xml = Get-Content $fullPath
    $ns = @{msb = "http://schemas.microsoft.com/developer/msbuild/2003" }
    
    # Track changes
    $changesMade = 0
    
    # Remove Win32 ProjectConfiguration elements
    $configNodes = Select-Xml -Xml $xml -XPath "//msb:ProjectConfiguration[msb:Platform='Win32']" -Namespace $ns
    foreach ($node in $configNodes) {
        $node.Node.ParentNode.RemoveChild($node.Node) | Out-Null
        $changesMade++
    }
    
    # Remove Win32 PropertyGroup elements
    $propGroups = Select-Xml -Xml $xml -XPath "//msb:PropertyGroup[contains(@Condition, 'Win32')]" -Namespace $ns
    foreach ($node in $propGroups) {
        $node.Node.ParentNode.RemoveChild($node.Node) | Out-Null
        $changesMade++
    }
    
    # Remove Win32 ItemDefinitionGroup elements
    $itemDefGroups = Select-Xml -Xml $xml -XPath "//msb:ItemDefinitionGroup[contains(@Condition, 'Win32')]" -Namespace $ns
    foreach ($node in $itemDefGroups) {
        $node.Node.ParentNode.RemoveChild($node.Node) | Out-Null
        $changesMade++
    }
    
    # Remove Win32 ImportGroup elements
    $importGroups = Select-Xml -Xml $xml -XPath "//msb:ImportGroup[contains(@Condition, 'Win32')]" -Namespace $ns
    foreach ($node in $importGroups) {
        $node.Node.ParentNode.RemoveChild($node.Node) | Out-Null
        $changesMade++
    }
    
    # Remove individual Condition attributes containing Win32 in ClCompile, Link, etc.
    $allNodes = Select-Xml -Xml $xml -XPath "//*[@Condition]" -Namespace $ns
    foreach ($node in $allNodes) {
        $condition = $node.Node.GetAttribute("Condition")
        if ($condition -and $condition.Contains("Win32")) {
            $node.Node.RemoveAttribute("Condition")
            $changesMade++
        }
    }
    
    # Replace WIN32 preprocessor definitions with _WIN64 in x64 configurations
    $preprocessorNodes = Select-Xml -Xml $xml -XPath "//msb:PreprocessorDefinitions[contains(text(), 'WIN32')]" -Namespace $ns
    foreach ($node in $preprocessorNodes) {
        if ($node.Node.InnerText.Contains("WIN32")) {
            # Check if this is in an x64 configuration
            $parent = $node.Node
            while ($parent -and -not $parent.GetAttribute("Condition")) {
                $parent = $parent.ParentNode
            }
            
            if ($parent -and $parent.GetAttribute("Condition") -match "x64") {
                # Replace WIN32 with _WIN64 for x64 configs
                $newText = $node.Node.InnerText -replace '\bWIN32\b', '_WIN64'
                $node.Node.InnerText = $newText
                $changesMade++
            }
        }
    }
    
    if ($changesMade -gt 0) {
        # Save the file
        $xml.Save($fullPath)
        Write-Host "  [OK] Removed $changesMade Win32 configuration items" -ForegroundColor Green
    } else {
        Write-Host "  [OK] No Win32 configurations found" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "[SUCCESS] All projects now configured for x64 only!" -ForegroundColor Green
Write-Host "Changes made:" -ForegroundColor Cyan
Write-Host "  - Removed Win32/x86 project configurations" -ForegroundColor White
Write-Host "  - Removed Win32 PropertyGroups and ItemDefinitionGroups" -ForegroundColor White
Write-Host "  - Updated preprocessor definitions for x64" -ForegroundColor White
