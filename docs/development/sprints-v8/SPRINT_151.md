# Sprint 151: Plugin Sandbox Policy Framework

**Block:** v8.3.0 — Phase P1: Plugin Ecosystem Hardening  
**Status:** ✅ Done  
**Sprint Count:** 151 / 174

---

## Overview

Defines the plugin sandboxing policy for DarkThumbs. Establishes permission tiers, resource
quotas (CPU time, memory, file I/O), and violation response strategies to ensure third-party
plugins cannot destabilize the host thumbnail process.

---

## Deliverables

| Artifact | Path | Notes |
|---|---|---|
| Header | `Engine/Plugin/PluginSandboxPolicy.h` | Permission tiers, quotas, violation responses |
| GTest | `Engine/Tests/Sprint151_PluginSandboxPolicy.cpp` | 13 test cases |
| Sprint doc | `docs/development/sprints-v8/SPRINT_151.md` | This document |

---

## Tests (13)

- `SandboxPolicy_BasicInstantiation`
- `SandboxPolicy_TieredPermissions` — Basic/Standard/Trusted/System tiers
- `SandboxPolicy_CPUQuota` — CPU time quota constant (100ms)
- `SandboxPolicy_MemoryQuota` — memory quota constant (64MB)
- `SandboxPolicy_FileIOQuota` — file I/O read-only restriction
- `SandboxPolicy_NetworkDenied` — network access denied by default
- `SandboxPolicy_ViolationResponse` — Log/Terminate/Quarantine responses
- `SandboxPolicy_TrustedTierElevated` — Trusted tier has higher quotas
- `SandboxPolicy_PolicyComparison` — stricter/less strict policy merge
- `SandboxPolicy_DefaultPolicyForTier` — DefaultPolicy() factory
- `SandboxPolicy_PolicySerialization` — policy to string representation
- `SandboxPolicy_BasicTierLimits` — Basic tier most restrictive
- `SandboxPolicy_SystemTierUnrestricted` — System tier = full access

---

## Acceptance Criteria

- [x] Header compiles with `/W4` zero warnings
- [x] 4 permission tiers defined
- [x] Resource quotas enforced at policy level
- [x] All 13 GTest cases pass
- [x] Sprint doc created

---

## Related Sprints

- Sprint 150: Plugin Runtime Test Matrix
- Sprint 154: Plugin Trust Chain
