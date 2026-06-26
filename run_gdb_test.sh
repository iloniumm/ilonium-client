#!/bin/bash
gdb -ex "run" -ex "backtrace" -ex "quit" --args ./src/armagetronad_main > gdb.log 2>&1 &
GDB_PID=$!

echo "Started gdb with PID $GDB_PID"
sleep 8 # Wait for local game to auto-start

echo "Sending Escape key..."
./send_key Escape
sleep 3

echo "Sending Escape key..."
./send_key Escape
sleep 3

if kill -0 $GDB_PID 2>/dev/null; then
    echo "gdb/game is still running. No crash caught by gdb."
    kill $GDB_PID
else
    echo "gdb/game exited/crashed! Log content:"
    cat gdb.log
fi
