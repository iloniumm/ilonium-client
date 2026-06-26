#!/bin/bash
# Run the game in the background with output redirected to game.log
./src/armagetronad_main > game.log 2>&1 &
GAME_PID=$!

echo "Started game with PID $GAME_PID (should auto-start local game)..."
sleep 8 # Wait for local game to load completely

echo "Sending Escape key to open in-game menu..."
./send_key Escape

sleep 3 # Wait 3 seconds to see if it crashes or remains running

if kill -0 $GAME_PID 2>/dev/null; then
    echo "Game is still running! No crash detected."
    kill $GAME_PID
else
    echo "Game crashed! Log content:"
    cat game.log
fi
