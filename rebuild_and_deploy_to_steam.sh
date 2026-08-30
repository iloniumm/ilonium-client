#!/bin/bash
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
: "${STEAMDIR:=$HOME/.local/share/Steam/steamapps/common/Retrocycles}"
set -e

echo "=========================================================="
echo " Starting Retrocycles Build & Steam Deployment Script"
echo "=========================================================="
echo "This script will rebuild the game client using Docker"
echo "and then deploy it directly to your Steam installation."
echo ""

cd "$ROOT"

# 1. Clean up target tags and tarball to force a full rebuild
sudo -S rm -f armagetronad-0.2.9.3.0-ilonas-modpack.tar.gz fingerprint .changetag
cd docker/build
sudo -S rm -rf steamdirs *.tag context.steam* result.winbuild_steam* result.appdir_*_steam* source source.tag

# 2. Build the Steam packages using Docker
echo ">>> Running Docker build..."
sudo -S env "PATH=$PATH:/usr/local/bin:/usr/bin:/bin" make steamdirs/steam_linux -j$(nproc)

# 3. Verify build output exists
cd "$ROOT"
BUILD_DIR="docker/build/steamdirs/steam_linux.dir"
if [ ! -d "$BUILD_DIR" ]; then
    echo "ERROR: Compiled directory $BUILD_DIR not found!"
    exit 1
fi

# 4. Copy build files to Snap Steam install path
STEAM_DIR=""$STEAMDIR""
if [ ! -d "$STEAM_DIR" ]; then
    echo "ERROR: Steam directory $STEAM_DIR not found!"
    exit 1
fi

echo ">>> Deploying compiled client to Steam..."
cp -rv "$BUILD_DIR"/* "$STEAM_DIR"/

echo "=========================================================="
echo " SUCCESS: Build and Steam deployment completed!"
echo " Please launch/restart Retrocycles in Steam now."
echo "=========================================================="
