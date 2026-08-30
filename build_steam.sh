#!/bin/bash
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
set -e
echo "Starting local Steam build (Windows & Linux)..."
cd "$ROOT"

echo "Fixing file permissions..."
sudo chown -R $USER:$USER "$ROOT"

echo "Configuring Git..."
git config --global --add safe.directory "$ROOT"
sudo git config --global --add safe.directory "$ROOT"

# Clean up previous target tags to force a rebuild of current source changes
cd docker/build
sudo rm -rf steamdirs *.tag context.steam* result.winbuild_steam* result.appdir_*_steam*

echo "Phase 1: Compiling Steam Builds (Windows & Linux) using Docker..."
# Run the build via sudo to allow docker access
sudo env "PATH=$PATH:/usr/local/bin:/usr/bin:/bin" make steam_windows steam_linux -j$(nproc)

echo "==================================="
echo "Done! The Steam packages are ready in:"
echo " - Linux:   "$ROOT"/docker/build/steamdirs/steam_linux/"
echo " - Windows: "$ROOT"/docker/build/steamdirs/steam_windows/"
echo "==================================="
