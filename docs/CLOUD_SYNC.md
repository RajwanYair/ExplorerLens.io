# Cloud Sync & Collaboration Support

ExplorerLens v18.2.0+ generates thumbnails for cloud-resident files across
**OneDrive, SharePoint, Azure Blob, and S3-compatible storage** — without
triggering full file downloads for large un-hydrated placeholders.

---

## Decision Pipeline

```
File thumbnail request
        │
        ▼
OfflineAvailabilityChecker.Check(path)
 ├── FullyLocal     → decode from local buffer (normal fast path)
 ├── PartialHydrate → CfApi header hydration (up to 128 KB)
 │                    → decode from hydrated bytes
 ├── CloudThumb     → CloudThumbnailFetcher.Fetch() from Graph API
 └── Skip           → metered network + non-local file → no thumbnail
        │
        ▼
Thumbnail rendered + CloudSyncStatusBadge.Composite()
        │
        ▼
CollaborationMarker.Composite() (if sharing info available)
        │
        ▼
HiDPIThumbnailCache.Put()
```

---

## Supported Cloud Providers

| Provider                  | Availability Check  | Thumbnail Fetch     | Sync Badge |
|---------------------------|---------------------|---------------------|------------|
| OneDrive Personal         | CfApi               | Graph API /thumbnails | Yes      |
| OneDrive for Business     | CfApi               | Graph API /thumbnails | Yes      |
| SharePoint Online         | CfApi / UNC         | SPO REST API        | Yes        |
| SharePoint Server (on-prem)| UNC                | SPO REST (limited)  | Partial    |
| Azure Blob Storage        | HTTP Range request  | N/A (direct decode) | No         |
| S3-Compatible             | HTTP Range request  | N/A (direct decode) | No         |

---

## Cloud Thumbnail Strategy

For un-hydrated placeholders, ExplorerLens prefers the **server-generated thumbnail**
from Microsoft Graph when available (avoids any local decode overhead):

```
GET https://graph.microsoft.com/v1.0/me/drive/items/{id}/thumbnails/0/large
```

The response is a JPEG byte stream, decoded to BGRA and stored in the HiDPI cache.

If Graph API is unavailable (no auth token, metered network), the fallback chain is:

1. **Header hydration** — hydrate first 128 KB via `CF_OPERATION_ACK_DATA` and probe the magic bytes
2. **Local decode** — decode from the available bytes (may result in partial image for very large unpacked formats)
3. **Skip** — return `S_FALSE` to Explorer; no thumbnail shown

---

## Sync Status Badges

The `CloudSyncStatusBadge` class reads `PKEY_StorageProviderState` shell property
(Windows 10 1709+) and composites a 28×28 overlay badge in the bottom-right corner:

| Status         | Badge    | Description                        |
|----------------|----------|------------------------------------|
| Synced         | ✓ Green  | Local copy matches cloud           |
| Uploading      | ↑ Blue   | Upload in progress                 |
| Downloading    | ↓ Blue   | Download in progress               |
| Conflict       | ✗ Red    | Version conflict detected          |
| Error          | ! Orange | Sync error (see StorageProvider)   |
| Paused         | ‖ Grey   | Sync paused by user or policy      |
| CloudOnly      | ☁ Grey   | Not downloaded; cloud placeholder  |
| PinnedLocal    | 📌 Green  | Always keep offline enabled        |

---

## Collaboration Badges

When an authenticated Graph API session is available, `CollaborationMarker`
fetches sharing info for the file and composites:

- A **people icon** if the file is shared with specific people
- A **lock icon** for files shared with the organisation
- A **globe icon** for publicly shared files
- A number disc (e.g. **"3"**) when 3+ users are actively co-editing

Collaboration info is cached per-item for 60 seconds to avoid Graph API throttling.

---

## Network-Aware Prefetch

`NetworkAwarePrefetcher` prioritises:
1. Local files (no bandwidth cost)
2. Partially-hydrated placeholders (low cost)
3. Cloud-only placeholders on unmetered connections
4. Cloud-only placeholders on metered connections (disabled by default)

**Default bandwidth budget:** 1 MB/s for cloud prefetch (configurable in `LENSManager`).

---

## Configuration (LENSManager → Cloud tab)

| Setting                      | Default | Description                                   |
|------------------------------|---------|-----------------------------------------------|
| Enable cloud thumbnails      | On      | Fetch Graph API thumbnails for cloud files    |
| Prefetch on metered network  | Off     | Allow cloud prefetch on cellular/metered      |
| Max hydration KB             | 128     | Header bytes to hydrate before giving up      |
| Cloud bandwidth budget MB/s  | 1       | Rate limit for background cloud prefetch      |
| Graph API token scope        | Files.Read | OAuth scope for thumbnail fetch            |
| Show sync status badge       | On      | Overlay sync state badge on thumbnails        |
| Show collaboration badge     | On      | Overlay sharing/co-author badge               |

---

## Security Notes

- **No credentials are stored** in ExplorerLens. The `CloudThumbnailFetcher` accepts
  short-lived bearer tokens obtained externally (via WAM / MSAL) and never persists them.
- **SAS URLs** for Azure/S3 are passed as opaque strings; ExplorerLens does not
  inspect or log the token portions.
- **Range requests** use HTTPS exclusively — HTTP range requests are rejected.
