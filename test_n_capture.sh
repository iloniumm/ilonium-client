#!/bin/bash
# test_n_capture.sh

export DISPLAY=:1

# Start the game
./src/armagetronad_main > game_test.log 2>&1 &
GAME_PID=$!

echo "Game started with PID $GAME_PID"
sleep 6

# Take main menu screenshot
import -window root /home/scarlet/.gemini/antigravity/brain/1e44e0a6-d43a-47df-adbf-0afe3fa26936/artifacts/main_menu_new.png
echo "Main menu screenshot captured"

# Press Enter to start a local game (or whatever key starts it)
# Wait, in the custom main menu:
# Let's see what keys can start the game.
# The user's custom main menu is built with ImGui.
# Can we press Enter or space, or use Tab/arrows?
# Let's send a mouse click or keys.
# Wait, we have send_key utility. Let's see how send_key is compiled and run.
# Let's first kill the game.
kill $GAME_PID
sleep 1
