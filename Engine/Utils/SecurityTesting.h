#pragma once
//==============================================================================
// ExplorerLens Engine — Unified Security Testing Module
//
// Consolidates: FuzzingEngine, SecurityCompliance, SecurityHardeningV2
//
// Provides fuzzing infrastructure, security hardening validation,
// ASLR/DEP/CFG checks, and compliance auditing.
//==============================================================================

// Fuzzing engine (Engine/Utils)
#include "FuzzingEngine.h"
#include "SecurityCompliance.h"

// Security hardening (Engine/Core)
#include "../Core/SecurityHardeningV2.h"
