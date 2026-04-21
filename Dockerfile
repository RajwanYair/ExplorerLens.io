# ExplorerLens LensServer — Windows Server Core container
# Copyright (c) 2026 ExplorerLens Project
#
# Build:
#   docker build -t explorerlens-server .
#
# Run:
#   docker run -p 8765:8765 -v C:\Photos:C:\Photos:ro explorerlens-server
#
# ENV vars:
#   LENS_PORT        Listen port (default 8765)
#   LENS_BIND        Bind address (default 0.0.0.0 in container)
#   LENS_GPU_OFF     Set to "1" to disable GPU (required in most CI containers)
#   LENS_MAX_SIZE    Maximum thumbnail size in px (default 1024)
#   LENS_VERBOSE     Set to "1" for request logging
#
ARG EXPLORERLENS_VERSION=36.3.0

# ── Stage 1: Build ─────────────────────────────────────────────────────────────
FROM mcr.microsoft.com/windows/servercore:ltsc2022 AS builder

SHELL ["powershell", "-NoProfile", "-Command"]

# Install Scoop (user-mode package manager, no admin required in container)
RUN Set-ExecutionPolicy RemoteSigned -Scope CurrentUser -Force; \
    $env:SCOOP='C:\scoop'; \
    [Environment]::SetEnvironmentVariable('SCOOP', $env:SCOOP, 'Machine'); \
    iwr get.scoop.sh -outfile 'install.ps1'; .\install.ps1 -RunAsAdmin

RUN scoop install cmake ninja git 7zip

# Install VS Build Tools 2026 (v145 toolset — smallest footprint, C++ only)
# NOTE: When VS 2026 BuildTools are available on mcr.microsoft.com, switch to
# the official container image. Until then, use the installer channel /vs/18/.
RUN curl -fsSL -o vs_buildtools.exe \
    'https://aka.ms/vs/18/release/vs_buildtools.exe' && \
    Start-Process vs_buildtools.exe -Wait -ArgumentList \
      '--quiet', '--wait', '--norestart', \
      '--add', 'Microsoft.VisualStudio.Workload.VCTools', \
      '--add', 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', \
      '--add', 'Microsoft.VisualStudio.Component.Windows11SDK.26100'

WORKDIR C:\\build
COPY . C:\\build

# Build LensServer
RUN cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\2026\BuildTools\VC\Auxiliary\Build\vcvars64.bat" && \
    cmake --preset default-release -DBUILD_TESTS=OFF -DBUILD_BENCHMARKS=OFF && \
    cmake --build --preset default-release --target LensServer -j 4'

# ── Stage 2: Runtime ─────────────────────────────────────────────────────────
FROM mcr.microsoft.com/windows/servercore:ltsc2022 AS runtime

ARG EXPLORERLENS_VERSION=36.3.0
LABEL org.opencontainers.image.title="ExplorerLens LensServer"
LABEL org.opencontainers.image.description="REST thumbnail server for ExplorerLens"
LABEL org.opencontainers.image.version="${EXPLORERLENS_VERSION}"
LABEL org.opencontainers.image.source="https://github.com/RajwanYair/ExplorerLens"

WORKDIR C:\\lens

COPY --from=builder C:\\build\\build\\bin\\LensServer.exe .
COPY --from=builder C:\\build\\x64\\Release\\LENSShell.dll .

ENV LENS_PORT=8765
ENV LENS_BIND=0.0.0.0
ENV LENS_GPU_OFF=1
ENV LENS_MAX_SIZE=1024
ENV LENS_VERBOSE=0

EXPOSE 8765

HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
  CMD powershell -Command \
    "try { $r = Invoke-WebRequest http://localhost:$($env:LENS_PORT)/health -UseBasicParsing; exit 0 } catch { exit 1 }"

ENTRYPOINT ["powershell", "-NoProfile", "-Command", \
  "& C:\\lens\\LensServer.exe \
     --port $env:LENS_PORT \
     --bind $env:LENS_BIND \
     --max-size $env:LENS_MAX_SIZE \
     $(if($env:LENS_GPU_OFF -eq '1'){'--gpu-off'}) \
     $(if($env:LENS_VERBOSE -eq '1'){'--verbose'})"]
