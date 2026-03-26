# AI-First Architecture

ExplorerLens v19.0.0+ introduces AI-powered post-processing for every thumbnail, using
lightweight ONNX models running on the DirectML GPU compute path or CPU fallback.
All AI stages are **opt-in** and gate on model availability — no model = no AI overhead.

---

## Pipeline Overview

```
Decoded BGRA pixels
        │
        ▼
AIThumbnailPipeline.Process()
 │
 ├── ContentCategoryClassifier  → "Photo" / "Document" / "Code" / ...
 │        └── Selects post-process hint (sharpen_text / saturation_boost / ...)
 │
 ├── SemanticColorPalette.Extract()  → Dominant 6-color palette for folder tiles
 │
 ├── SmartCropAnalyzer (existing)  → Saliency-guided crop region
 │
 ├── BlurDetectionFilter.Detect()
 │        └── if blurScore > 0.35 → DeblurMethod::UnsharpMask or AIDeblur
 │
 ├── NSFWContentGuard (enterprise only)
 │        └── flagged → blur / replace / block according to policy
 │
 └── NeuralThumbnailSynthesizer (synthesis fallback)
          └── Only if no real decode pixels available (corrupt / encrypted file)
        │
        ▼
AIPipelineResult.pixels ── HiDPI scale ── CloudSyncBadge ── CollabMarker ── Cache
```

---

## AI Modules

### ContentCategoryClassifier

A MobileNetV3-Small model trained on a 13-class taxonomy:

| Class        | Post-Process Hint        |
|--------------|--------------------------|
| Photo        | `saturation_boost`       |
| Document     | `sharpen_text`           |
| Code         | `sharpen_text`           |
| Diagram      | `edge_enhance`           |
| Art          | `none`                   |
| Medical      | `grayscale_normalize`    |
| Spreadsheet  | `sharpen_text`           |
| Video frame  | `deinterlace`            |

Model size: ~2 MB. Inference: ~1.5 ms on DirectML, ~8 ms CPU.

### BlurDetectionFilter

Two-stage pipeline:
1. **Laplacian Variance** (CPU, <0.2ms) — quick sharpness score
2. If score < threshold → optional **AI deblur** (RRDB-lite, ~4ms GPU)

### SemanticColorPalette

k-means++ palette extraction (CPU path, no model needed):
- 6 dominant colors with coverage percentages
- Optional Material Design color name labelling
- CIE Lab distance for perceptually accurate grouping
- Encoded as compact hex strings in the thumbnail cache metadata

### NeuralThumbnailSynthesizer

A small diffusion-lite model conditioned on filename tokens, extension, MIME type,
and file size class. Generates a 256×256 representative placeholder thumbnail when:
- File decoding fails entirely (corrupt / encrypted)
- Format has no registered decoder and no plugin

### NSFWContentGuard *(Enterprise only)*

Requires an enterprise license key. Runs a binary safety classifier:
- `Monitor`: Log detections, no alteration
- `BlurOnDetect`: Gaussian blur (σ=12) overlay
- `ReplaceOnDetect`: Swap with a neutral document icon
- `BlockOnDetect`: Return S_FALSE → Explorer shows generic icon

---

## Model Management

All ONNX models are stored in `%LOCALAPPDATA%\ExplorerLens\models\`:

```
models/
├── classifier-v2.onnx          ContentCategoryClassifier
├── blur-detector-v1.onnx       BlurDetectionFilter (deblur path)
├── synthesizer-v1.onnx         NeuralThumbnailSynthesizer
├── smart-crop-v3.onnx          SmartCropAnalyzer
├── nsfw-guard-v1.onnx          NSFWContentGuard (enterprise)
└── search-embed-v2.onnx        SearchEmbeddingGenerator
```

`AIModelRegistry` scans this directory at startup and hot-swaps models on update.

---

## Performance Budget

Total AI budget per thumbnail: **5 ms** (leaves 12 ms for decode within 17 ms SLO).

| Module               | p95 (GPU) | p95 (CPU) | Enabled by default |
|----------------------|-----------|-----------|--------------------|
| ContentClassifier    | 1.5 ms    | 8 ms      | Yes                |
| SemanticColorPalette | 0.4 ms    | 0.4 ms    | Yes                |
| SmartCrop            | 1.2 ms    | 6 ms      | Yes                |
| BlurDetect (fast)    | 0.2 ms    | 0.2 ms    | Yes                |
| AI Deblur            | 3.5 ms    | 45 ms     | No (threshold gate) |
| NSFWGuard            | 1.8 ms    | 12 ms     | No (enterprise)    |
| Synthesizer          | 22 ms     | 180 ms    | Fallback only      |

`AIPerformanceProfiler.RecommendDisable()` automatically suggests which stages to
disable if measured p95 latency exceeds the configured budget on the current device.

---

## Configuration

All AI features are configurable in **LENSManager → AI** tab:

| Setting                     | Default | Description                           |
|-----------------------------|---------|---------------------------------------|
| Enable content classification | On   | Run MobileNetV3 content classifier    |
| Enable color palette         | On    | k-means++ palette extraction          |
| Enable smart crop            | On    | Saliency-guided crop                  |
| Enable blur detection        | On    | Laplacian variance sharpness check    |
| Enable AI deblur             | Off   | RRDB-lite deblur (high latency)       |
| NSFW guard mode              | Off   | Disabled by default (enterprise only) |
| Prefer GPU for AI            | On    | Use DirectML; fallback to CPU         |
| AI budget ms                 | 5.0   | Max total AI time per thumbnail       |
