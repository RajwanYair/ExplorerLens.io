# lens-autocomplete.ps1 — PowerShell Argument Completer for lens.exe
# Copyright (c) 2026 ExplorerLens Project
#
# Sprint 23 (v15.4.0 "Zenith-U"): Registers a tab-completion handler for
# the 'lens' command so that subcommands and options complete automatically
# in PowerShell 7+ and Windows PowerShell 5.1+.
#
# Usage:
#   . "$PSScriptRoot\lens-autocomplete.ps1"
#   lens <TAB>               -> complete subcommands
#   lens generate --<TAB>    -> complete generate options
#   lens cache <TAB>         -> complete cache actions
#

$_subcommands = @(
    'benchmark', 'cache', 'doctor', 'generate', 'info',
    'register', 'unregister'
)

$_subcommandHelp = @{
    'benchmark'  = 'Measure thumbnail decode throughput and latency'
    'cache'      = 'Manage the ExplorerLens thumbnail cache'
    'doctor'     = 'Run system health checks for ExplorerLens'
    'generate'   = 'Generate thumbnail(s) for one or more files'
    'info'       = 'Show format detection and metadata for a file'
    'register'   = 'Register the ExplorerLens shell extension (admin)'
    'unregister' = 'Unregister the ExplorerLens shell extension (admin)'
}

$_optionsBySubcommand = @{
    'generate'   = @('--output', '--size', '--quality', '--format', '--recursive', '--verbose', '--json')
    'info'       = @('--json', '--verbose')
    'cache'      = @('clear', 'stats', 'warm', '--verbose', '--json')
    'register'   = @('--status', '--dll', '--verbose')
    'unregister' = @('--dll', '--verbose')
    'benchmark'  = @('--corpus', '--iterations', '--json', '--verbose')
    'doctor'     = @('--json', '--verbose')
}

$_globalOptions = @('--help', '--version', '--verbose', '--json', '-v', '-j', '-h')

Register-ArgumentCompleter -Native -CommandName 'lens', 'lens.exe' -ScriptBlock {
    param($wordToComplete, $commandAst, $cursorPosition)

    $tokens = $commandAst.CommandElements | Select-Object -Skip 1

    # Determine the active subcommand (first non-option token after 'lens')
    $subcommand = $null
    foreach ($tok in $tokens) {
        $val = $tok.Value
        if ($val -and $val[0] -ne '-' -and $val -in $_subcommands) {
            $subcommand = $val
            break
        }
    }

    if ($null -eq $subcommand) {
        # Complete subcommand name
        $_subcommands | Where-Object { $_ -like "$wordToComplete*" } | ForEach-Object {
            $help = $_subcommandHelp[$_]
            [System.Management.Automation.CompletionResult]::new(
                $_, $_, 'ParameterValue', $help)
        }
        $_globalOptions | Where-Object { $_ -like "$wordToComplete*" } | ForEach-Object {
            [System.Management.Automation.CompletionResult]::new(
                $_, $_, 'ParameterName', $_)
        }
        return
    }

    # Complete options for the identified subcommand
    $opts = $_optionsBySubcommand[$subcommand]
    if ($opts) {
        $opts | Where-Object { $_ -like "$wordToComplete*" } | ForEach-Object {
            [System.Management.Automation.CompletionResult]::new(
                $_, $_, 'ParameterName', $_)
        }
    }

    # File path completion for subcommands that take a file argument
    if ($subcommand -in @('generate', 'info')) {
        Get-ChildItem -Path "$wordToComplete*" -ErrorAction SilentlyContinue |
            ForEach-Object {
                $completion = $_.FullName
                $label      = $_.Name
                [System.Management.Automation.CompletionResult]::new(
                    "`"$completion`"", $label, 'ProviderItem', $_.FullName)
            }
    }
}

Write-Host "lens.exe PowerShell argument completer registered." -ForegroundColor Green
