# PowerShell Build Environment Setup

## Overview
PowerShell is now configured to automatically load Visual Studio Build Tools 2022 environment on startup.

## Profile Location
```
C:\Users\ryair\OneDrive - Intel Corporation\Documents\WindowsPowerShell\Microsoft.PowerShell_profile.ps1
```

## What's Loaded
The profile automatically runs `vcvars64.bat` and imports all Visual Studio environment variables, making these tools immediately available:

- **cl** - C/C++ Compiler (MSVC 19.50.35717)
- **msbuild** - Build Engine (18.0.2)
- **nmake** - Make utility
- **lib** - Library manager
- **link** - Linker
- **dumpbin** - Binary analyzer
- **rc** - Resource compiler
- **mt** - Manifest tool

## Verified Commands
```powershell
cl                    # Microsoft C/C++ Optimizing Compiler Version 19.50.35717 for x64
msbuild /version      # MSBuild version 18.0.2+995a3dce4 for .NET Framework
nmake /?              # Usage: NMAKE @commandfile
lib /?                # Library manager
dumpbin /?            # Binary dump utility
```

## How It Works
1. On PowerShell startup, profile checks for vcvars64.bat
2. Runs vcvars64.bat in a temporary batch file
3. Captures all environment variables (PATH, INCLUDE, LIB, etc.)
4. Imports them into the PowerShell session
5. Displays confirmation: "Visual Studio Build Tools 2022 environment loaded"

## VS Installation Path
```
C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\
```

## vcvars64.bat Location
```
C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat
```

## Benefits
- **No manual initialization required** - Open PowerShell and build tools are ready
- **Seamless workflow** - Switch between terminals without re-running setup scripts
- **Consistent environment** - Same configuration every time
- **Auto-updates** - If VS Build Tools PATH changes, profile adapts automatically

## Testing
Open a fresh PowerShell window and run:
```powershell
cl          # Should show compiler version
msbuild     # Should show MSBuild help
nmake       # Should show nmake usage
```

## Customization
To customize the profile, edit:
```powershell
notepad $PROFILE
```

Or in VS Code:
```powershell
code $PROFILE
```

## Optional: Set Default Directory
Uncomment the last line in the profile to automatically navigate to the DarkThumbs project:
```powershell
cd "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs"
```

## Troubleshooting

### Profile Not Running
If the profile doesn't load, check PowerShell execution policy:
```powershell
Get-ExecutionPolicy
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Build Tools Not Found
Verify VS Build Tools installation:
```cmd
dir "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
```

### Profile Location
Check profile path:
```powershell
echo $PROFILE
Test-Path $PROFILE
```

## Created
2025-01-19 - Configured for DarkThumbs static linking development
