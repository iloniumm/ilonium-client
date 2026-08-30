#!/bin/bash
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
: "${STEAMDIR:=$HOME/.local/share/Steam/steamapps/common/Retrocycles}"
set -e
echo "Compiling Retrocycles locally..."
cd "$ROOT"/src
make -j$(nproc)

echo "Deploying to Steam..."
STEAM_BIN=""$STEAMDIR"/usr/bin"
cp -f --remove-destination armagetronad_main ${STEAM_BIN}/Retrocycles
cp -f --remove-destination armagetronad_main ${STEAM_BIN}/armagetronad
cp -f --remove-destination armagetronad_main ${STEAM_BIN}/retrocycles
echo "Build and deploy complete!"
