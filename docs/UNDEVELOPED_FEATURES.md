# Undeveloped Features & Plans

**Date**: January 8, 2026  
**Source**: Analysis of ROADMAP.md and project documentation  
**Current Version**: v5.3.0 (build complete, ready for testing)

---

## 📊 SUMMARY

**Total Roadmap Items**: ~40 major features  
**Completed**: 11 items (28%)  
**In Progress**: 2 items (5%)  
**Not Started**: 27 items (67%)

**Current Phase**: Phase 1 (Foundation & Stability) - 70% complete

---

## ✅ COMPLETED FEATURES (What We Have)

### Build System & Libraries (Phase 1 - Priority 0)
- ✅ **zlib 1.3.1** - Built and integrated
- ✅ **LZ4 1.10.0** - Built and integrated
- ✅ **zstd 1.5.7** - Built and integrated
- ✅ **minizip-ng 4.0.10** - Built and integrated
- ✅ **libwebp 1.5.0** - Built and integrated
- ✅ **LZMA SDK 24.08** - Built and integrated (NEW: 7z/XZ support)
- ✅ **UnRAR DLL 7.2.2** - Built and integrated (NEW: RAR support)
- ✅ **Main project** - CBXShell.dll and CBXManager.exe build successfully

### Installation & Testing (Phase 1 - Priority 1)
- ✅ **Installation script** - Automated installer with COM registration
- ✅ **File locking fix** - Explorer restart to release DLL locks
- ✅ **Testing guide** - Comprehensive installation testing documentation

### Archive Format Support
- ✅ **ZIP/CBZ** - Minizip-ng integration
- ✅ **RAR/CBR** - UnRAR DLL integration (NEW)
- ✅ **7z/CB7** - LZMA SDK integration (NEW)
- ✅ **XZ** - LZMA SDK support (NEW)
- ✅ **LZ4** - LZ4 library integration
- ✅ **ZSTD** - Zstd library integration

---

## ⏳ IN PROGRESS (Current Work)

### Phase 1: Foundation & Stability

#### Installation Testing (P1 - 50% complete)
- ✅ Installation script working
- ✅ COM registration automated
- ✅ Testing guide created
- ⏳ **PENDING**: User testing with admin privileges
- ❌ **NOT DONE**: Silent installation mode
- ❌ **NOT DONE**: Unattended installation for CI/CD

#### Build System Optimization (P0 - 80% complete)
- ✅ All libraries build successfully
- ✅ Individual library build scripts
- ⏳ **PARTIAL**: Automated "build from scratch" single command
- ❌ **NOT DONE**: Build time optimization (<10 minutes target, currently ~54 seconds for main, unclear for full)

---

## ❌ NOT STARTED (Major Features)

### Phase 1: Foundation & Stability (Jan-Feb 2026)

#### P2: Advanced Image Formats
**Status**: 🔴 **NOT STARTED** (requires meson setup)  
**Dependencies**: meson build system not configured

- ❌ **dav1d** (AV1 video decoder)
  - **Complexity**: Medium - requires meson
  - **Value**: High - enables AVIF support
  - **Effort**: 2-4 hours
  
- ❌ **libavif 1.3.0** (AVIF image format)
  - **Complexity**: Medium - depends on dav1d
  - **Value**: High - modern image format
  - **Effort**: 2-3 hours
  
- ❌ **libjxl 0.11.1** (JPEG XL)
  - **Complexity**: HIGH - many dependencies (brotli, highway, lcms2)
  - **Value**: Medium - emerging format
  - **Effort**: 4-8 hours
  
- ❌ **HEIF/HEIC** support
  - **Complexity**: High - requires libheif + dependencies
  - **Value**: High - Apple Photos format
  - **Effort**: 4-6 hours

#### P3: Installation Enhancements
- ❌ **Silent installation** mode for automated deployment
- ❌ **Unattended installation** for CI/CD pipelines
- ❌ **Update mechanism** for new versions
- ❌ **Multi-user installation** support

---

### Phase 2: Performance & Quality (Mar-May 2026)

#### P4: Performance Optimization
**Status**: 🔴 **NOT STARTED**  
**Value**: High - user experience improvement

- ❌ **GPU texture pooling**
  - Pre-allocate GPU textures to avoid allocation overhead
  - Spec exists: `docs/TEXTURE_POOLING.md`
  - Estimated impact: 20-30% performance improvement
  
- ❌ **Multi-threaded decoding**
  - Parallelize image decompression
  - Batch processing for multiple thumbnails
  - Estimated impact: 2-3x throughput improvement
  
- ❌ **Cache optimization**
  - Intelligent cache eviction policies
  - Predictive pre-loading
  - Estimated impact: Faster repeat access
  
- ❌ **Background thumbnail generation**
  - Non-blocking thumbnail creation
  - Priority queue for visible items
  - Estimated impact: Better UI responsiveness

#### P5: Observability & Diagnostics
**Status**: 🔴 **NOT STARTED**  
**Specs exist**: Multiple specs in docs/  
**Value**: Medium - debugging and optimization

- ❌ **Performance metrics collection**
  - Spec: `docs/OBSERVABILITY_SPEC_V1.md`
  - Spec: `docs/PERFORMANCE_METRICS.md`
  - Timing data for all operations
  - GPU utilization metrics
  - Cache hit rates
  
- ❌ **Telemetry system**
  - Anonymous usage data collection
  - Error reporting
  - Performance baselines
  
- ❌ **Diagnostics dashboard**
  - Real-time performance view
  - Cache statistics
  - Format usage tracking
  
- ❌ **Performance regression gates**
  - Spec: `docs/PERF_REGRESSION_GATES.md`
  - Automated performance testing
  - Prevent performance degradation in CI/CD

---

### Phase 3: Extensibility (Jun-Aug 2026)

#### P6: Plugin System Architecture
**Status**: 🔴 **NOT STARTED**  
**Complexity**: VERY HIGH  
**Value**: VERY HIGH - enables community contributions

- ❌ **Plugin SDK**
  - Spec: `docs/SDK_GUIDE.md` (partially complete)
  - Spec: `docs/PLUGIN_PACKAGE_FORMAT_V1.md`
  - C++ API for format providers
  - Sample plugins
  - Plugin development guide
  
- ❌ **Plugin security sandbox**
  - Spec: `docs/PLUGIN_SANDBOX_MODEL_V1.md`
  - Spec: `docs/SANDBOX_MODEL.md`
  - AppContainer isolation
  - Resource limits (CPU, memory, disk)
  - Capability-based security
  - Plugin crash isolation
  
- ❌ **Plugin loading and lifecycle**
  - Dynamic plugin discovery
  - Version compatibility checking
  - Plugin initialization/shutdown
  - Hot-reload support (for development)
  
- ❌ **Code signing requirements**
  - Spec: `docs/CODE_SIGNING.md`
  - Authenticode verification
  - Publisher trust database
  - Certificate chain validation
  - Revocation checking (CRL/OCSP)

#### P7: Plugin Marketplace
**Status**: 🔴 **NOT STARTED**  
**Complexity**: HIGH  
**Value**: HIGH - community ecosystem

- ❌ **Marketplace protocol**
  - Spec: `docs/MARKETPLACE_PROTOCOL.md`
  - Spec: `docs/PLUGIN_MARKETPLACE_PROTOCOL_V1.md`
  - Plugin repository specification
  - Package metadata format
  - Update checking mechanism
  
- ❌ **Plugin distribution**
  - GitHub-based hosting
  - Automated publishing workflow
  - Version management
  - Plugin compatibility validation
  
- ❌ **Plugin discovery UI**
  - Gallery view in CBXManager
  - Search and filtering
  - Install/uninstall/update UI
  - Plugin ratings and reviews
  
- ❌ **Community plugin gallery**
  - Curated plugin collection
  - Quality badges
  - Download statistics
  - User reviews

---

### Phase 4: Modernization (Sep-Dec 2026)

#### P8: Engine Refactoring
**Status**: 🔴 **NOT STARTED**  
**Complexity**: VERY HIGH  
**Value**: HIGH - code quality and maintainability

- ❌ **Modular architecture**
  - Spec: `docs/P2_ENGINE_REFACTORING_PLAN.md`
  - Separate engine from shell extension
  - Interface standardization
  - Dependency injection
  
- ❌ **CMake build system**
  - Replace Visual Studio projects
  - Cross-platform potential
  - Better dependency management
  
- ❌ **Compatibility kit**
  - Spec: `docs/COMPATIBILITY_KIT_SPEC.md`
  - Format testing framework
  - Automated format validation
  - Regression test suite

#### P9: CI/CD Pipeline
**Status**: 🔴 **NOT STARTED**  
**Value**: HIGH - development velocity

- ❌ **GitHub Actions integration**
  - Automated builds on push
  - Multi-configuration testing
  - Artifact publishing
  
- ❌ **Automated testing**
  - Spec: `docs/TEST_STRATEGY_V1.md`
  - Unit tests
  - Integration tests
  - Format compatibility tests
  - Performance benchmarks
  
- ❌ **Release automation**
  - Spec: `docs/DEPLOYMENT_CHECKLIST.md`
  - Automated version bumping
  - Release notes generation
  - Installer creation
  - Code signing
  
- ❌ **Code signing infrastructure**
  - Spec: `docs/CODE_SIGNING.md`
  - Certificate management
  - Timestamp server
  - Signing automation

---

### Phase 5: Enterprise & Distribution (2027+)

#### P10: Enterprise Features
**Status**: 🔴 **NOT STARTED**  
**Value**: Medium - enterprise adoption

- ❌ **Group Policy support**
  - Centralized configuration
  - Format enable/disable policies
  - Cache size limits
  - Security policies
  
- ❌ **Centralized plugin management**
  - Admin-approved plugin whitelist
  - Corporate plugin repository
  - Forced plugin updates
  
- ❌ **Audit logging**
  - File access tracking
  - Plugin usage logging
  - Compliance reporting
  
- ❌ **Multi-GPU optimizations**
  - Spec: `docs/MULTI_GPU_TESTING_GUIDE.md`
  - Automatic GPU selection
  - Load balancing across GPUs
  - Hybrid Intel/NVIDIA/AMD support

#### P11: Next-Generation Features (v7.0+)
**Status**: 🔴 **NOT STARTED**  
**Timeframe**: 2027  
**Complexity**: EXPERIMENTAL

- ❌ **Cloud-based thumbnail rendering**
  - Offload to cloud for heavy formats
  - Caching service
  - API for mobile/web clients
  
- ❌ **Machine learning format detection**
  - AI-based format identification
  - Damaged file recovery
  - Smart format conversion
  
- ❌ **Predictive caching**
  - ML-based access patterns
  - Pre-generate likely-needed thumbnails
  - User behavior learning
  
- ❌ **Cross-device sync**
  - Thumbnail cache synchronization
  - Settings sync across machines
  - Cloud storage integration
  
- ❌ **Web-based management portal**
  - Remote configuration
  - Statistics dashboard
  - Plugin management

---

## 📋 PRIORITIZED NEXT STEPS

### Immediate (Next 1-2 weeks)

1. **Complete Phase 1 P1 - Installation Testing**
   - User testing with admin privileges
   - Validate RAR/7z support works
   - Performance baseline measurement
   
2. **Phase 1 P0 - Build System Optimization**
   - Create single "build all" script
   - Measure and optimize build time
   - Document build process

### Near-term (Next 1-2 months)

3. **Phase 1 P2 - Advanced Image Formats** (if desired)
   - Setup meson build system
   - Build dav1d (AV1 decoder)
   - Build libavif (AVIF support)
   - Consider libjxl (JPEG XL) based on adoption

4. **Phase 1 P3 - Installation Enhancements**
   - Silent installation mode
   - Update mechanism
   - Multi-user support

### Medium-term (Next 3-6 months)

5. **Phase 2 P4 - Performance Optimization**
   - GPU texture pooling (highest impact)
   - Multi-threaded decoding
   - Cache optimization

6. **Phase 2 P5 - Observability**
   - Performance metrics collection
   - Diagnostics tools
   - Performance regression testing

### Long-term (6-12 months)

7. **Phase 3 P6 - Plugin System**
   - Plugin SDK design and implementation
   - Security sandbox model
   - Code signing infrastructure

8. **Phase 3 P7 - Plugin Marketplace**
   - Marketplace protocol
   - Plugin discovery UI
   - Community gallery

---

## 🎯 EFFORT ESTIMATES

### Quick Wins (1-4 hours)
- Silent installation mode
- Build system "build all" script
- Performance metrics collection (basic)

### Small Projects (1-3 days)
- dav1d build (with meson)
- libavif integration
- Multi-user installation
- Cache optimization
- Diagnostics dashboard (basic)

### Medium Projects (1-2 weeks)
- GPU texture pooling
- Multi-threaded decoding
- libjxl integration (complex dependencies)
- Automated testing infrastructure
- Performance regression gates

### Large Projects (1-3 months)
- Plugin SDK v1.0
- Security sandbox implementation
- Plugin marketplace protocol
- Engine refactoring
- CI/CD pipeline

### Very Large Projects (3-6 months)
- Full plugin system with marketplace
- Enterprise features suite
- CMake migration
- Cloud rendering infrastructure

---

## 💡 STRATEGIC RECOMMENDATIONS

### Option 1: Polish & Release (Conservative)
**Focus**: Complete Phase 1, user testing, performance optimization  
**Timeframe**: 1-2 months  
**Outcome**: Stable v5.3.0 release with 6 archive formats

**Deliverables**:
- ✅ Installation tested and validated
- ✅ Performance baseline established
- ✅ Basic observability/metrics
- ✅ Documentation complete
- ✅ Release-ready installer

**Skip**: Advanced image formats, plugin system, enterprise features

### Option 2: Add Modern Image Formats (Moderate)
**Focus**: Phase 1 + Phase 1 P2 (dav1d, libavif, libjxl)  
**Timeframe**: 2-3 months  
**Outcome**: v5.4.0 with AVIF, HEIF, JPEG XL support

**Deliverables**:
- ✅ Everything from Option 1
- ✅ AVIF support (libavif + dav1d)
- ✅ HEIF/HEIC support (libheif)
- ⚠️ JPEG XL support (libjxl - complex, may skip)

**Skip**: Plugin system, enterprise features

### Option 3: Full Roadmap (Ambitious)
**Focus**: Execute full Phase 1-3 roadmap  
**Timeframe**: 6-12 months  
**Outcome**: v6.0.0 with plugin system and marketplace

**Deliverables**:
- ✅ Everything from Option 2
- ✅ Plugin SDK v1.0
- ✅ Security sandbox
- ✅ Plugin marketplace
- ✅ Performance optimizations
- ✅ Observability suite

**Skip**: Enterprise features, cloud rendering, ML features

---

## 📊 CURRENT STATUS SUMMARY

### What Works Now ✅
- **6 compression formats**: ZIP, RAR, 7z, XZ, LZ4, ZSTD
- **3 image formats**: WebP, JPEG, PNG (and others via WIC)
- **Archives**: CBZ, CBR, CB7 comic book formats
- **Build system**: All libraries build successfully
- **Installation**: Automated installer with COM registration
- **Documentation**: Comprehensive guides and references

### What's Missing ❌
- **Modern image formats**: AVIF, HEIF, JPEG XL (not yet built)
- **Plugin system**: No extensibility framework
- **Performance**: No texture pooling, single-threaded decoding
- **Observability**: No metrics, no diagnostics dashboard
- **Testing**: Installation requires manual admin testing
- **CI/CD**: No automated builds/tests
- **Enterprise**: No GP support, no centralized management

### What's Partially Done ⏳
- **Build system**: Works but not optimized (<10 min goal)
- **Installation**: Works but lacks silent mode
- **Testing**: Guide exists but not executed
- **Documentation**: Many specs written but features not implemented

---

## 🔗 RELATED DOCUMENTATION

**Comprehensive Analysis**:
- [CONSOLIDATION_ANALYSIS_2026-01-08.md](CONSOLIDATION_ANALYSIS_2026-01-08.md) - Full project consolidation report

**Current Status**:
- [READY_FOR_TESTING.md](../READY_FOR_TESTING.md) - Current build status and testing steps

**Roadmap**:
- [ROADMAP.md](../ROADMAP.md) - Full development roadmap

**Build Information**:
- [BUILD_GUIDE.md](BUILD_GUIDE.md) - Build instructions
- [BUILD_SCRIPTS_REFERENCE.md](BUILD_SCRIPTS_REFERENCE.md) - Script reference
- [LIBRARY_BUILD_PROGRESS_2026-01-08.md](LIBRARY_BUILD_PROGRESS_2026-01-08.md) - Library status

**Architecture Specs** (Not Implemented):
- [P2_ENGINE_REFACTORING_PLAN.md](P2_ENGINE_REFACTORING_PLAN.md) - Engine refactoring
- [PLUGIN_SANDBOX_MODEL_V1.md](PLUGIN_SANDBOX_MODEL_V1.md) - Plugin security
- [MARKETPLACE_PROTOCOL.md](MARKETPLACE_PROTOCOL.md) - Plugin marketplace
- [OBSERVABILITY_SPEC_V1.md](OBSERVABILITY_SPEC_V1.md) - Observability system
- [PERFORMANCE_METRICS.md](PERFORMANCE_METRICS.md) - Metrics collection

---

**Summary**: 28% of planned features complete. Phase 1 (Foundation) is 70% done. Immediate focus should be installation testing and build optimization. Advanced features (plugins, performance, enterprise) require significant additional development (6-12 months).
