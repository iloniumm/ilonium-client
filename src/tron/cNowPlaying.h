// What is playing, asked of the system the game is running on.
//
// The widget used to depend on a shell script that somebody had to start by
// hand, so for everyone who did not know about it there was never anything to
// show. Each system can be asked directly; this is where that happens.

#ifndef RC_NOWPLAYING_H
#define RC_NOWPLAYING_H

#include <string>

struct rc_Playing
{
    bool        playing;
    float       position;   //!< seconds into the track
    float       length;     //!< seconds the track runs for
    std::string title;
    std::string artist;
    std::string album;
    std::string who;        //!< the player that answered, to send orders back to

    rc_Playing() : playing( false ), position( 0.0f ), length( 0.0f ) {}
};

//! Tells the player what to do: -1 previous, 0 play/pause, 1 next.
//! @return false when there was nobody to tell.
bool rc_TellPlayer( int what );

//! Seeks to a position in seconds. False when the system offers no way.
bool rc_SeekPlayer( float seconds );

//! Asks whatever this system offers. Rate limited internally, so it is safe to
//! call from a polling loop.
//! @return false when nothing could be learned, leaving out untouched.
bool rc_AskNowPlaying( rc_Playing & out );

#endif
