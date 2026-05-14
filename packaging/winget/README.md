# ExplorerLens — winget package manifest

Three-file winget manifest for submission to
[winget-pkgs](https://github.com/microsoft/winget-pkgs).

## Files

| File | Purpose |
| ------ | --------- |
| `ExplorerLens.ExplorerLens.yaml` | Version manifest — identifies the package |
| `ExplorerLens.ExplorerLens.installer.yaml` | Installer manifest — MSI URL + SHA256 + product code |
| `ExplorerLens.ExplorerLens.locale.en-US.yaml` | Locale manifest — publisher/description metadata |

## Publishing Workflow

1. Build and upload the MSI to the v39.9.0 GitHub Release.
1. Compute the SHA-256 of `ExplorerLens-Setup-39.9.0.msi`:

   ```powershell
   (Get-FileHash packaging/output/ExplorerLens-Setup-39.9.0.msi -Algorithm SHA256).Hash
   ```

1. Replace `InstallerSha256` in `ExplorerLens.ExplorerLens.installer.yaml`
   with the real hash.
1. Validate locally:

   ```powershell
   winget validate --manifest packaging/winget
   ```

1. Fork [winget-pkgs](https://github.com/microsoft/winget-pkgs), copy the
   three files to `manifests/e/ExplorerLens/ExplorerLens/39.9.0/`, open a PR.

## Updating for a new version

Run `build-scripts/Bump-Version.ps1 -Version <X.Y.Z>` which will refresh all
three files in place. Regenerate the MSI SHA-256 and re-submit the PR.

## Roadmap reference

- ROADMAP v6.0 §14 — distribution channels table (winget mandatory Phase 1)
- ROADMAP v6.0 §18 Phase 1 — winget package manifest + PR
