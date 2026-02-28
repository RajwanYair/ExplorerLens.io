#pragma once
//==============================================================================
// ExplorerLens Engine — Unified Accessibility Module
//
// Consolidates: AccessibilityEngine, AccessibilitySuiteV2,
//               AccessibilityPipeline, AccessibilityNarratorBridge
//
// Provides accessibility compliance checking, screen reader support,
// narrator bridge, and WCAG/Section 508 validation.
//==============================================================================

// Core accessibility engine (Engine/Utils)
#include "../Utils/AccessibilityEngine.h"

// Accessibility suite and pipeline (Engine/Core)
#include "AccessibilityPipeline.h"
#include "AccessibilitySuiteV2.h"
#include "AccessibilityNarratorBridge.h"
