#!/bin/bash
# Скрипт для удобного запуска модифицированного клиента RetroCycles
# с использованием ресурсов из установленной Steam-версии.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

DATADIR="${SCRIPT_DIR}/usr/share/games/Retrocycles"
if [ ! -d "$DATADIR" ]; then
    DATADIR="${SCRIPT_DIR}/../share/games/Retrocycles"
fi
if [ ! -d "$DATADIR" ]; then
    DATADIR="$HOME/snap/steam/common/.local/share/Steam/steamapps/common/Retrocycles/usr/share/games/Retrocycles"
fi

SYSCONFIGDIR="${SCRIPT_DIR}/usr/etc/games/Retrocycles"
if [ ! -d "$SYSCONFIGDIR" ]; then
    SYSCONFIGDIR="${SCRIPT_DIR}/../etc/games/Retrocycles"
fi
if [ ! -d "$SYSCONFIGDIR" ]; then
    SYSCONFIGDIR="$HOME/snap/steam/common/.local/share/Steam/steamapps/common/Retrocycles/usr/etc/games/Retrocycles"
fi

CONFIGDIR="/tmp/retrocycles_client_$$" # Уникальная папка конфигов для каждого окна
DATADIRTMP="/tmp/retrocycles_data_$$"

echo "Запускаем модифицированный клиент..."
echo "Ресурсы берутся из: $DATADIR"

mkdir -p "$CONFIGDIR/var"
if [ -f "$HOME/.Retrocycles/var/user.cfg" ]; then
    cp "$HOME/.Retrocycles/var/user.cfg" "$CONFIGDIR/var/user.cfg"
fi
# Copy recent demos so they show up in the menu!
cp "$HOME/snap/steam/common/.Retrocycles/var/"*.aarec "$CONFIGDIR/var/" 2>/dev/null || true

cd "${SCRIPT_DIR}"
./armagetronad_main --datadir "$DATADIR" --configdir "$SYSCONFIGDIR" --userconfigdir "$CONFIGDIR" --userdatadir "$DATADIRTMP" --vardir "$CONFIGDIR/var"
