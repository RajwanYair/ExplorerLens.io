#pragma once
//==============================================================================
// ExplorerLens Engine — Unified Version Management
//
// Consolidates: VersionDriftDetector, VersionDriftGate, VersionSynchronizer,
//               VersionSyncV14, LibraryVersionAudit, LibraryInventoryManager
//
// Single public include for all version tracking, drift detection,
// synchronization, and library audit functionality.
//==============================================================================

// Version authority — single source of truth for current version
#include "VersionSynchronizer.h"

// Drift detection — CI-integrated version consistency checking
#include "VersionDriftDetector.h"
#include "VersionDriftGate.h"

// Library tracking — external library version audit
#include "LibraryInventoryManager.h"
#include "LibraryVersionAudit.h"

// Historical baseline
#include "VersionSyncV14.h"
