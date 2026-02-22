# Sprint 313: Plugin Performance Profiler

**Status:** ✅ Complete
**Component:** `Engine/Plugin/PluginPerformanceProfiler.h`
**Tests:** 5 (TestPluginPerf_MetricNames, TestPluginPerf_AlertNames, TestPluginPerf_SamplingNames, TestPluginPerf_MetricCount, TestPluginPerf_AlertCount)

## Overview
Per-plugin performance sampling and alerting to detect and report decode latency regressions, memory leaks, and CPU monopolisation.

## Key Features
- PluginPerfMetric: DecodeLatencyMs, MemoryDeltaKB, CPUPercent, GPUPercent, ThreadCount, ExceptionCount (6 metrics)
- PluginPerfAlert: SlowDecode, MemoryLeak, HighCPU, HighGPU, TooManyThreads, Crash
- PluginPerfSamplingRate: Off, Low (1 Hz), Medium (10 Hz), High (100 Hz), Continuous
- Profiling data stored in lock-free ring buffer; exported to ETW and JSON
