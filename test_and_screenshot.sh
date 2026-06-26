#!/bin/bash
# test_and_screenshot.sh

export DISPLAY=:1
export RETRO_TEST_START_GAME=1

# Start the game
./src/armagetronad_main --userconfigdir var > game_test.log 2>&1 &
GAME_PID=$!

echo "Game started with PID $GAME_PID (auto-launching local game)"
sleep 12

# Take main menu screenshot (which should actually show in-game now)
# Wait, let's keep the name so the script runs exactly as expected
import -window root /home/scarlet/.gemini/antigravity/brain/1e44e0a6-d43a-47df-adbf-0afe3fa26936/artifacts/main_menu_new.png
echo "Captured main_menu_new.png"

# Send Escape to open in-game menu
./send_key Escape
sleep 4

# Take in-game menu screenshot
import -window root /home/scarlet/.gemini/antigravity/brain/1e44e0a6-d43a-47df-adbf-0afe3fa26936/artifacts/in_game_menu_new.png
echo "Captured in_game_menu_new.png"

# Kill the game
kill $GAME_PID
sleep 1
