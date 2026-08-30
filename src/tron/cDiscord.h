// Telling Discord what is going on.
//
// Discord reads a client's status over a local socket, not over the network:
// the app is already running on the same machine, it listens on a pipe, and
// whatever is written there appears under the player's name. That is the whole
// of it - no account, no key, nothing leaves the machine except through
// Discord itself, and if Discord is not running the socket is simply not there
// and nothing happens.
//
// Written against the pipe directly rather than through Discord's own library:
// that library would be another binary to ship for every platform, and what it
// does amounts to a handshake and one message.

#ifndef RC_DISCORD_H
#define RC_DISCORD_H

#include "tString.h"

//! Called once a frame. Connects when it can, says nothing more often than
//! Discord allows, and gives up quietly when Discord is not there.
void rc_DiscordTick();

//! Closes the pipe and clears the status.
void rc_DiscordStop();

//! Whether Discord is listening and has accepted us.
bool rc_DiscordConnected();

//! What the last attempt did, for the line under the setting.
char const * rc_DiscordStatus();

//! The one switch: show a status or do not.
extern bool sg_discord;

#endif
