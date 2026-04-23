---
mode: agent
description: "Fetch the official format specification for a given image/archive/document format and summarize the decoder-relevant sections: magic bytes, header layout, colorspace handling, and known edge cases."
---

# Format Spec Fetch

Fetch the authoritative specification for **{{format}}** and extract the sections most relevant to implementing a thumbnail decoder.

## Context Files

- `.github/instructions/decoder-authoring.instructions.md`
- `.github/skills/decoder-development/SKILL.md`
- `Engine/Decoders/` — existing decoder implementations for reference patterns

## Task

1. **Identify the authoritative spec source** for `{{format}}`. Use one of:
   - ISO/IEC spec document (HEIF → ISO/IEC 23008-12, JXL → ISO/IEC 18181, AVIF → AOM AV1 Image File Format)
   - IETF RFC (WebP → RFC-nnnn)
   - Format-specific official docs (PNG → libpng.org, PDF → adobe.com/devnet/pdf)
   - GitHub reference implementation docs (AVIF → aomedia.googlesource.com, JXL → libjxl.readthedocs.io)

2. **Fetch and summarize** these spec sections:

   ### 2a. Magic Bytes & Detection
   - Byte offsets 0–32 identifying the format
   - Multi-extension ambiguity (e.g., `.heic` vs `.heif` vs `.hif`)
   - How `FormatDetector` (ROADMAP §7.1 [H12]) should detect this format

   ### 2b. File Structure Overview
   - Top-level container structure (boxes, chunks, atoms)
   - Where the pixel data lives vs. metadata
   - Mandatory vs. optional sections

   ### 2c. Thumbnail / Preview Extraction Fast Path
   - Any embedded preview/thumbnail (EXIF thumbnail, `hnti` box in HEIC, LibRaw embedded JPEG [H7])
   - Whether `ProbeHeader()` (IStreamingDecoder, ROADMAP §7.4) can return metadata from first 16 KB alone
   - Partial decode support (progressive JPEG, truncated PNG, etc.)

   ### 2d. Colorspace & HDR
   - Colorspace signaling (ICC profile location, NCLX box, CICP parameters)
   - HDR / wide-gamut handling (PQ/HLG → SDR tone-mapping requirements)
   - Alpha channel presence and premultiplied vs. straight alpha

   ### 2e. Edge Cases & Security Considerations
   - Known parser vulnerabilities or CVEs in this format
   - Dimensions/bit-depth overflow risks (SafeInt usage, ROADMAP §15.1)
   - Animated variants requiring loop count handling
   - Multi-page / multi-frame considerations

3. **Output a decoder spec summary** in this structure:

   ```markdown
   ## {{FORMAT}} Decoder Spec Summary

   **Authoritative source:** [link]
   **Library:** [recommended library from §8.3 ROADMAP]
   **Tier:** P0 / P1 / P2 / P3 (§7.3 ROADMAP)

   ### Magic bytes
   | Offset | Bytes | Notes |
   |--------|-------|-------|

   ### ProbeHeader() contract
   - Min bytes needed: N KB
   - Returns from probe: width, height, bit-depth, has-alpha, has-embedded-preview

   ### Fast preview path
   - [description of embedded thumbnail extraction]

   ### Colorspace
   - [ICC / NCLX / CICP handling]

   ### HDR tone-map required
   - Yes / No — [details]

   ### Known edge cases
   - [list]

   ### Security notes
   - [CVE references, overflow points]
   ```

4. **Cross-check against** `Engine/Decoders/` to see if a decoder exists; if so, note any spec divergences in the existing implementation.

## Output

Produce the decoder spec summary and a list of actionable TODOs for the `{{format}}` decoder implementation or hardening.
