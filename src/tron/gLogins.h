// Who the server said is logged in, and as what.
//
// A client is never told this directly. The player sync carries the name, the
// ping, the score and the team, and nothing about authentication - the
// authenticated name and the access level are server-side fields that never
// go over the wire. So a client that wants to show logins has exactly one
// source: the server announces every login and logout in the console, where
// everybody can read them.
//
// That is what this reads. It is not guesswork - the announcement is the
// server's own statement, in the same words it uses for the log - but it does
// mean the record only goes back as far as the moment you connected. Somebody
// who logged in before you arrived is not in it, and cannot be.

#ifndef RC_GLOGINS_H
#define RC_GLOGINS_H

#include "defs.h"

//! Reads whatever the console has said since the last call.
//!
//! Cheap when nothing has been said, which is almost always - it compares a
//! line count and returns.
void rc_ScanLogins();

//! What the given player is logged in as, or nothing if the server never said.
//!
//! The name is matched with colour codes stripped from both sides, because the
//! announcement carries them and the player list does not always.
char const * rc_LoginOf( char const * playerName );

//! Forgets everything. A different server has different people on it.
void rc_ForgetLogins();


//! Reads the login list the server publishes in its info reply.
//!
//! Covers everybody on the server, including those who logged in before we
//! connected, which the console announcements cannot.
void rc_LoginsFromServer();

#endif
