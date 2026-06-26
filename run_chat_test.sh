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

# Send Return to bypass the First Setup wizard / start game
./send_key Return
sleep 3

# Wait for local game to load completely
sleep 5

# Ensure US layout is active initially
setxkbmap us

# Send Return to open chat
./send_key Return
sleep 2

# Switch keyboard layout to Russian (ru)
echo "Switching layout to RU..."
setxkbmap ru
sleep 1

# Send keys corresponding to Cyrillic characters
echo "Sending Cyrillic keys..."
./send_key Cyrillic_pe
sleep 0.2
./send_key Cyrillic_er
sleep 0.2
./send_key Cyrillic_o
sleep 0.2
./send_key Cyrillic_el
sleep 0.2
./send_key Cyrillic_de
sleep 0.2

# Switch back to US layout
echo "Switching layout back to US..."
setxkbmap us
sleep 1

# Take screenshot of the chat input
import -window root /home/scarlet/.gemini/antigravity/brain/2357c39d-c5eb-4dd6-81b3-e42f7011b3a1/artifacts/chat_cyrillic_test.png
echo "Captured chat_cyrillic_test.png"

# Send Return to submit the chat
./send_key Return
sleep 2

# Take a screenshot to show the chat bubble or chat history
import -window root /home/scarlet/.gemini/antigravity/brain/2357c39d-c5eb-4dd6-81b3-e42f7011b3a1/artifacts/chat_submitted.png
echo "Captured chat_submitted.png"

# Kill the game
kill $GAME_PID
sleep 1
