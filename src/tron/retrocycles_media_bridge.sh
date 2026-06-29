#!/bin/bash
# =====================================================================
# Retrocycles Media Player Bridge Daemon
# Bypasses Snap sandbox restrictions using a simple file-based IPC.
# =====================================================================

TARGET_FILE="/home/scarlet/snap/steam/common/.armagetronad/retrocycles_media_state.txt"
CMD_FILE="/home/scarlet/snap/steam/common/.armagetronad/retrocycles_media_cmd.txt"

mkdir -p "$(dirname "$TARGET_FILE")"

# Initialize files
echo "" > "$CMD_FILE"

echo "Retrocycles Media Bridge started."
echo "State file: $TARGET_FILE"
echo "Command file: $CMD_FILE"

while true; do
    # 1. Process commands written by the game
    if [ -s "$CMD_FILE" ]; then
        cmd=$(cat "$CMD_FILE" | tr -d '\r\n[:space:]')
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
            # Clear command file
            echo "" > "$CMD_FILE"
        fi
    fi

    # 2. Query playerctl and write state
    raw=$(/usr/bin/playerctl metadata --format '{{status}}|{{position}}|{{mpris:length}}|{{title}}|{{artist}}|{{album}}' 2>/dev/null)
    if [ -z "$raw" ]; then
        # Check if playerctl is running but just doesn't have metadata (or player paused/inactive)
        status=$(/usr/bin/playerctl status 2>/dev/null)
        if [ "$status" = "Paused" ] || [ "$status" = "Playing" ]; then
            title=$(/usr/bin/playerctl metadata title 2>/dev/null)
            artist=$(/usr/bin/playerctl metadata artist 2>/dev/null)
            album=$(/usr/bin/playerctl metadata album 2>/dev/null)
            pos=$(/usr/bin/playerctl position 2>/dev/null | tr -d '\r\n')
            # position is returned in seconds by "playerctl position", multiply by 1000000 to match microsecond format
            if [ ! -z "$pos" ]; then
                pos_us=$(echo "$pos * 1000000" | bc -l 2>/dev/null | cut -d'.' -f1)
            else
                pos_us="0"
            fi
            len=$(/usr/bin/playerctl metadata mpris:length 2>/dev/null)
            if [ -z "$len" ]; then len="0"; fi
            if [ -z "$title" ]; then title="Unknown Title"; fi
            
            echo "$status|$pos_us|$len|$title|$artist|$album" > "$TARGET_FILE"
        else
            echo "Stopped|0|0|No media playing||" > "$TARGET_FILE"
        fi
    else
        echo "$raw" > "$TARGET_FILE"
    fi

    sleep 0.25
done
