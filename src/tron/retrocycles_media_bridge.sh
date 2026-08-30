#!/bin/bash
# =====================================================================
# Retrocycles Media Player Bridge Daemon
# Bypasses Snap sandbox restrictions using a simple file-based IPC.
# Supports both .Retrocycles and .armagetronad folders under Steam/Home.
# =====================================================================

USER_HOME=$(eval echo ~${SUDO_USER:-$USER})

# Potential directories where the game might read/write IPC files
# Two of these running at once is worse than none: they answer a quarter of a
# second apart and take turns writing, so the time in the file walks backwards.
# The second one leaves quietly.
exec 9> "${TMPDIR:-/tmp}/retrocycles-media-bridge-$(id -u).lock"
if ! flock -n 9; then
    exit 0
fi

DIRS=(
    "$USER_HOME/snap/steam/common/.Retrocycles/var"
    "$USER_HOME/snap/steam/common/.armagetronad/var"
    "$USER_HOME/.Retrocycles/var"
    "$USER_HOME/.armagetronad/var"
)

# Ensure at least one directory exists to create files
mkdir -p "$USER_HOME/snap/steam/common/.Retrocycles/var"
mkdir -p "$USER_HOME/snap/steam/common/.armagetronad/var"

echo "Retrocycles Media Bridge started."
for d in "${DIRS[@]}"; do
    echo "Monitoring directory: $d"
    mkdir -p "$d" 2>/dev/null
    echo "" > "$d/retrocycles_media_cmd.txt"
done

while true; do
    # 1. Process commands written by the game (check all monitored directories)
    cmd=""
    for d in "${DIRS[@]}"; do
        if [ -s "$d/retrocycles_media_cmd.txt" ]; then
            cmd=$(cat "$d/retrocycles_media_cmd.txt" | tr -d '\r\n[:space:]')
            if [ ! -z "$cmd" ]; then
                # Clear cmd file in all directories to prevent duplicate execution
                for d2 in "${DIRS[@]}"; do
                    echo "" > "$d2/retrocycles_media_cmd.txt"
                done
                break
            fi
        fi
    done

    if [ ! -z "$cmd" ]; then
        echo "Executing command: $cmd"
        if [ "$cmd" = "prev" ]; then
            /usr/bin/playerctl previous 2>/dev/null
        elif [ "$cmd" = "play-pause" ]; then
            /usr/bin/playerctl play-pause 2>/dev/null
        elif [ "$cmd" = "next" ]; then
            /usr/bin/playerctl next 2>/dev/null
        elif [[ "$cmd" == seek* ]]; then
            seekSec=$(echo "$cmd" | sed 's/seek//')
            /usr/bin/playerctl position "$seekSec" 2>/dev/null
        fi
    fi

    # 2. Query playerctl and format metadata
    raw=$(/usr/bin/playerctl metadata --format '{{status}}|{{position}}|{{mpris:length}}|{{title}}|{{artist}}|{{album}}' 2>/dev/null)
    formatted_state=""
    if [ -z "$raw" ]; then
        status=$(/usr/bin/playerctl status 2>/dev/null)
        if [ "$status" = "Paused" ] || [ "$status" = "Playing" ]; then
            title=$(/usr/bin/playerctl metadata title 2>/dev/null)
            artist=$(/usr/bin/playerctl metadata artist 2>/dev/null)
            album=$(/usr/bin/playerctl metadata album 2>/dev/null)
            pos=$(/usr/bin/playerctl position 2>/dev/null | tr -d '\r\n')
            if [ ! -z "$pos" ]; then
                pos_us=$(echo "$pos * 1000000" | bc -l 2>/dev/null | cut -d'.' -f1)
            else
                pos_us="0"
            fi
            len=$(/usr/bin/playerctl metadata mpris:length 2>/dev/null)
            if [ -z "$len" ]; then len="0"; fi
            if [ -z "$title" ]; then title="Unknown Title"; fi
            
            formatted_state="$status|$pos_us|$len|$title|$artist|$album"
        else
            formatted_state="Stopped|0|0|No media playing||"
        fi
    else
        formatted_state="$raw"
    fi

    # 3. Write state to all active directories
    # Written aside and moved into place. Writing straight to the file empties
    # it first, and a reader landing in that gap saw nothing playing - which is
    # what made the track blink out for a moment every so often.
    for d in "${DIRS[@]}"; do
        if [ -d "$d" ]; then
            printf '%s\n' "$formatted_state" > "$d/retrocycles_media_state.txt.new" &&
                mv -f "$d/retrocycles_media_state.txt.new" "$d/retrocycles_media_state.txt"
        fi
    done

    sleep 0.25
done
