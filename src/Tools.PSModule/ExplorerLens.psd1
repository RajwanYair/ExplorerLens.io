@{
    # Script module or binary module file associated with this manifest.
    RootModule        = 'ExplorerLens.PS.dll'

    # Version number of this module.
    ModuleVersion     = '6.0.0'

    # Supported PSEditions
    # CompatiblePSEditions = @('Desktop', 'Core')

    # ID used to uniquely identify this module
    GUID              = 'e1a2b3c4-d5e6-f7a8-b9c0-d1e2f3a4b5c6'

    # Author of this module
    Author            = 'ExplorerLens.io Team'

    # Company or vendor of this module
    CompanyName       = 'ExplorerLens.io Project'

    # Copyright statement for this module
    Copyright         = '(c) 2026 ExplorerLens.io Project. All rights reserved.'

    # Description of the functionality provided by this module
    Description       = 'PowerShell module for managing ExplorerLens engine, plugins, and generating thumbnails.'

    # Minimum version of the PowerShell engine required by this module
    PowerShellVersion = '5.1'

    # Modules that must be imported into the global environment prior to importing this module
    # RequiredModules = @()

    # Assemblies that must be loaded prior to importing this module
    # RequiredAssemblies = @()

    # Script files (.ps1) that are run in the caller's environment prior to importing this module.
    # ScriptsToProcess = @()

    # Type files (.ps1xml) to be loaded when importing this module
    # TypesToProcess = @()

    # Format files (.ps1xml) to be loaded when importing this module
    # FormatsToProcess = @()

    # Lists the modules included in this module bundle
    # NestedModules = @()

    # Functions to export from this module
    # FunctionsToExport = '*'

    # Cmdlets to export from this module
    CmdletsToExport   = @(
        'Get-ExplorerLensInfo',
        'New-Thumbnail',
        'Get-ExplorerLensPlugin',
        'Install-ExplorerLensPlugin',
        'Remove-ExplorerLensPlugin',
        'Clear-ExplorerLensCache'
    )

    # Variables to export from this module
    VariablesToExport = '*'

    # Aliases to export from this module
    AliasesToExport   = '*'

    # DPI specific private data
    PrivateData       = @{
        PSData = @{
            # Tags applied to this module. These help with module discovery in online galleries.
            Tags       = @('Thumbnail', 'ShellExtension', 'Management', 'Plugins')
            
            # A URL to the license for this module.
            LicenseUri = 'https://github.com/ExplorerLensio/ExplorerLens.io/blob/main/LICENSE'
            
            # A URL to the project website for this module.
            ProjectUri = 'https://explorerlens.io'
            
            # A URL to an icon representing this module.
            # IconUri = ''
            
            # ReleaseNotes of this module
            # ReleaseNotes = ''
        }
    }
}
