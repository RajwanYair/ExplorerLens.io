# Sprint 287: NIfTI Neuroimaging Decoder

**Status:** ✅ Complete  
**Version:** v12.0  

## Objective
NIfTI (.nii/.nii.gz) neuroimaging format decoder for brain scan thumbnails.

## Deliverables
- `Engine/Decoders/NIfTIDecoder.h` — 7 data types, 3 slice orientations, 4 variants
- NIfTI-1 magic detection at offset 344 ("n+1\0" or "ni1\0")
- Axial/coronal/sagittal slice selection with windowing
- 5 unit tests: data types, slices, variants, counts, header defaults
