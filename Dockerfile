# ExplorerLens — SDK Development Container
#
# Provides the ExplorerLens C++ plugin headers and SDK documentation in a
# minimal Ubuntu environment, suitable for CI pipelines that build plugins
# or tools against the ExplorerLens SDK.
#
# Note: lens.exe, LENSShell.dll, and LENSManager.exe are Windows-only binaries.
# Download them from: https://github.com/RajwanYair/ExplorerLens.io/releases
#
# Usage:
#   docker run --rm ghcr.io/rajwanyair/explorerlens:latest
#   docker run --rm -v $(pwd):/work ghcr.io/rajwanyair/explorerlens:latest bash

FROM ubuntu:24.04

ARG EXPLORERLENS_VERSION=32.3.0

LABEL org.opencontainers.image.title="ExplorerLens SDK" \
      org.opencontainers.image.description="GPU-accelerated thumbnail provider SDK — 200+ file formats" \
      org.opencontainers.image.url="https://github.com/RajwanYair/ExplorerLens.io" \
      org.opencontainers.image.source="https://github.com/RajwanYair/ExplorerLens.io" \
      org.opencontainers.image.licenses="MIT" \
      org.opencontainers.image.version="${EXPLORERLENS_VERSION}"

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential \
      cmake \
      ninja-build \
      git \
      curl \
      ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /explorerlens

# SDK header tree
COPY SDK/include/           ./include/
COPY SDK/plugin_api.h       ./include/plugin_api.h
COPY SDK/PublicSDKSurface.h ./include/PublicSDKSurface.h
COPY SDK/PluginSDKv2.h      ./include/PluginSDKv2.h
COPY SDK/docs/              ./docs/
COPY SDK/examples/          ./examples/
COPY SDK/README.md          ./README_SDK.md

# Repository documentation
COPY README.md    ./
COPY CHANGELOG.md ./
COPY LICENSE      ./

ENV EXPLORERLENS_INCLUDE=/explorerlens/include
ENV EXPLORERLENS_VERSION=${EXPLORERLENS_VERSION}

CMD ["bash", "-c", \
     "echo \"ExplorerLens SDK v${EXPLORERLENS_VERSION} — headers at $EXPLORERLENS_INCLUDE\"; bash"]
