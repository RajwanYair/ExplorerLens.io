// resource_ids.h — String Resource ID Definitions
// Copyright (c) 2026 ExplorerLens Project
//
// Central header for all Win32 STRINGTABLE resource IDs.
// Include this in all .rc files and in C++ code that loads strings.
//
#pragma once

// Application identity
#define IDS_APP_NAME            1000
#define IDS_APP_TAGLINE         1001
#define IDS_APP_VERSION_FMT     1002

// Status
#define IDS_STATUS_REGISTERED   1010
#define IDS_STATUS_UNREGISTERED 1011
#define IDS_STATUS_PROCESSING   1012
#define IDS_STATUS_IDLE         1013
#define IDS_STATUS_ERROR        1014

// Dashboard
#define IDS_DASH_HIT_RATE       1020
#define IDS_DASH_THUMBNAILS     1021
#define IDS_DASH_GPU_BACKEND    1022
#define IDS_DASH_THROUGHPUT     1023

// Actions
#define IDS_ACTION_ENABLE       1030
#define IDS_ACTION_DISABLE      1031
#define IDS_ACTION_CLEAR_CACHE  1032
#define IDS_ACTION_OPEN_MGR     1033
#define IDS_ACTION_REGISTER     1034
#define IDS_ACTION_UNREGISTER   1035

// GPU backends
#define IDS_GPU_AUTO            1040
#define IDS_GPU_DX12            1041
#define IDS_GPU_DX11            1042
#define IDS_GPU_VULKAN          1043
#define IDS_GPU_CPU             1044

// Settings labels
#define IDS_SETTING_QUALITY     1050
#define IDS_SETTING_CACHE_SIZE  1051
#define IDS_SETTING_MAX_ENTRIES 1052
#define IDS_SETTING_THEME       1053
#define IDS_SETTING_TELEMETRY   1054

// Errors
#define IDS_ERR_REG_ADMIN       1060
#define IDS_ERR_DLL_NOT_FOUND   1061
#define IDS_ERR_GPU_INIT        1062
#define IDS_ERR_CACHE_CORRUPT   1063
#define IDS_ERR_PLUGIN_TRUST    1064
