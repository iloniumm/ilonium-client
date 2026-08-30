// Talking to the client's own backend.
//
// There is no login here and nothing to remember. The first time the game runs
// it makes a key pair, keeps the private half beside its configuration, and
// signs every request with it. The server knows the public half and therefore
// knows it is the same person as yesterday - without anyone typing a name, and
// without a password to lose, leak or store.
//
// Tags, mutes, bans and the wait between tickets all belong to the server. The
// client asks and shows the answer; it has no way of arguing with it, which is
// the whole point.

#ifndef RC_ILONIUM_H
#define RC_ILONIUM_H

#include <string>
#include <vector>

//! Makes the key if there is not one yet. Safe to call more than once.
bool rc_IloniumReady();

//! This installation's public key, base64. The name the server knows us by.
std::string rc_IloniumPublicKey();

//! Why the last request did not happen, in words. Shown to whoever is looking
//! at the chat, so a machine that is not this one can still be diagnosed.
std::string rc_IloniumWhy();

//! A signed request. Returns false only when the request could not be made at
//! all; an answer from the server, including a refusal, comes back as true with
//! the code and body filled in.
bool rc_IloniumCall( char const * method, std::string const & path,
                     std::string const & body, long & code, std::string & out );

//! One line of the chat, as the server tells it.
struct IloniumLine
{
    long long   id = 0;
    std::string name;
    std::string message;
    long long   stamp = 0;
    std::string roleTag;
    std::string roleColor;
};

//! What the server says about us: tag, mute, and the wait before another ticket.
struct IloniumSelf
{
    std::string roleTag;
    std::string roleColor;
    bool        muted = false;
    long long   muteUntil = 0;
    std::string muteReason;
    int         ticketWait = 0;     //!< seconds; 0 means a ticket can go now
    bool        chatBanned = false;
    std::string chatBanReason;
    bool        tickBanned = false;
    std::string tickBanReason;
    bool        known = false;      //!< false until the server has answered once
};

bool rc_IloniumFetch( std::vector< IloniumLine > & out, std::string & error );
bool rc_IloniumSay( std::string const & name, std::string const & text, std::string & error );
//! What the machine says about itself, sent with a ticket. The difference
//! between "it crashes" and something anyone can act on.
struct IloniumMachine
{
    std::string os;
    std::string cpu;
    std::string gpu;
    std::string ram;
    std::string disk;
};

bool rc_IloniumTicket( std::string const & name, std::string const & title,
                       std::string const & body, IloniumMachine const & box,
                       std::string & error );

//! The last thing the server said about us, refreshed by the calls above.
IloniumSelf const & rc_IloniumMe();

//! Asks the server about us right now. Used by screens that open cold.
bool rc_IloniumRefreshMe( std::string & error );

#endif
