#!/bin/bash
# Script to run client, take screenshot of the window, and clean up.

cd "$(dirname "$0")"

echo "Launching retrocycles in background..."
./launch_client.sh &
PID=$!

echo "Waiting for window to initialize..."
sleep 10

echo "Listing X11 windows..."
WID=$(xwininfo -root -children | grep -i "\"Retrocycles\"" | head -n 1 | awk '{print $1}')
echo "Found Retrocycles window ID: $WID"

if [ -n "$WID" ]; then
    echo "Capturing Retrocycles window screenshot..."
    import -window "$WID" /home/scarlet/.gemini/antigravity/brain/2357c39d-c5eb-4dd6-81b3-e42f7011b3a1/screenshot_main.png
else
    echo "Window not found, capturing root screenshot..."
    import -window root /home/scarlet/.gemini/antigravity/brain/2357c39d-c5eb-4dd6-81b3-e42f7011b3a1/screenshot_main.png
fi

sleep 2

echo "Killing client..."
kill $PID
wait $PID 2>/dev/null

echo "Done!"
