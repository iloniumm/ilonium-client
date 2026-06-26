#!/bin/bash
export DISPLAY=:1
export RETRO_TEST_START_GAME=1

# Clean up previous background game instances
pkill -f armagetronad_main || true
sleep 1

# Start the game
./src/armagetronad_main -f --userconfigdir var > game_test.log 2>&1 &
GAME_PID=$!

echo "Game started with PID $GAME_PID (auto-launching local game)"
sleep 7

# Send Return to bypass the First Setup wizard
./send_key Return
sleep 3

# Wait for local game to load completely
sleep 5

# Take in-game HUD screenshot
import -window root /home/scarlet/.gemini/antigravity/brain/2357c39d-c5eb-4dd6-81b3-e42f7011b3a1/artifacts/in_game_view.png
echo "Captured in_game_view.png"

# Send Escape to open in-game menu
./send_key Escape
sleep 4

# Take in-game menu screenshot
import -window root /home/scarlet/.gemini/antigravity/brain/2357c39d-c5eb-4dd6-81b3-e42f7011b3a1/artifacts/menu_visuals_tab.png
echo "Captured menu_visuals_tab.png"

# Kill the game
kill $GAME_PID
sleep 1
