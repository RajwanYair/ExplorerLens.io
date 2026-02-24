# ExplorerLens Test Corpus

This directory contains a comprehensive test file collection for validating decoder functionality.

## Structure

- **images/** - Standard and modern image formats (JPEG, PNG, WebP, AVIF, HEIF, JXL, etc.)
- **raw/** - Camera RAW formats (CR2, NEF, ARW, DNG)
- **archives/** - Archive formats (ZIP, CBZ, RAR, 7Z)
- **documents/** - Document formats (PDF, EPUB)
- **media/** - Audio/video with thumbnails (MP3, FLAC, MP4)
- **fonts/** - Font files (TTF, OTF)
- **models/** - 3D model formats (OBJ, STL)
- **svg/** - Scalable Vector Graphics

## Invalid Files

Each format category contains an invalid/ subdirectory with:
- **truncated** - File with only header bytes
- **empty** - Zero-byte file
- **garbage** - Random bytes with valid extension

These files test decoder robustness and crash resistance.

## Usage

Run the validator tool to test all formats:

```powershell
.\tools\ExplorerLensValidator\x64\Release\ExplorerLensValidator.exe tests\data\corpus -v -o results.csv
```

## Maintenance

To refresh the corpus with new test files:

```powershell
.\tests\Organize-TestCorpus.ps1 -CreateInvalidFiles
```

## Test Requirements (Sprint 13)

- ✅ At least 1 valid file per supported format
- ✅ Invalid/corrupt files for fuzzing
- ✅ Performance baseline files (various sizes)
- ✅ Real-world samples (not synthetic)

**Last Updated:** February 17, 2026  
**Version:** v7.0.0

