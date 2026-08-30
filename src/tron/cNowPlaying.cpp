#include "cNowPlaying.h"
#include "cShell.h"
#include "tConfiguration.h"
#include "defs.h"
#include "tSysTime.h"
#include "tConsole.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#if defined( _WIN32 ) || defined( WIN32 )
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

// says out loud what each attempt returned, so a machine that will not talk
// can be looked at instead of guessed about
bool rc_mediaReport = false;
static tConfItem<bool> rc_mediaReportConf( "RC_MEDIA_REPORT", rc_mediaReport );

namespace
{

// Asking costs a process on two of the three systems, so it is asked rarely
// and the answer is kept until it goes stale.
REAL       lastAsked = -1000;
rc_Playing lastAnswer;
bool       lastGood = false;

REAL const askEvery = 1.0f;

std::vector< std::string > Split( std::string const & line, char by )
{
    std::vector< std::string > parts;
    size_t start = 0;
    for ( size_t i = 0; i <= line.size(); ++i )
    {
        if ( i == line.size() || line[i] == by )
        {
            parts.push_back( line.substr( start, i - start ) );
            start = i + 1;
        }
    }
    return parts;
}

void Trim( std::string & text )
{
    while ( !text.empty() && ( text[ text.size() - 1 ] == '\n' ||
                               text[ text.size() - 1 ] == '\r' ||
                               text[ text.size() - 1 ] == ' ' ) )
        text.erase( text.size() - 1 );
    while ( !text.empty() && ( text[0] == ' ' ) )
        text.erase( 0, 1 );
}

#if !defined( __APPLE__ ) && !defined( _WIN32 ) && !defined( WIN32 )

//! Asks for a single piece and tells an answer apart from a complaint.
bool Field( std::string const & who, char const * what, std::string & value )
{
    value.clear();

    std::string answer;
    rc_RunHidden( "playerctl" + who + " " + what + " 2>&1", answer );
    Trim( answer );

    if ( answer.empty() || answer.find( "No player" ) != std::string::npos )
        return false;

    value = answer;
    return true;
}

//! Gives an order. Silence means it was carried out: the tool prints nothing
//! when it worked and complains when there was nobody to carry it out. So the
//! empty answer that means failure for a question means success for an order.
bool Tell( std::string const & who, std::string const & what )
{
    std::string answer;
    rc_RunHidden( "playerctl" + who + " " + what + " 2>&1", answer );
    Trim( answer );

    return answer.find( "No player" ) == std::string::npos &&
           answer.find( "not found" ) == std::string::npos;
}

#endif

#if defined( __APPLE__ )

//! Spotify and Music both answer questions put to them this way, and the tool
//! that puts them is part of the system.
bool AskPlayer( char const * app, rc_Playing & out )
{
    std::string script =
        "osascript -e 'if application \"";
    script += app;
    script += "\" is running then tell application \"";
    script += app;
    script += "\" to return (player state as text) & \"|\" & "
              "(player position as text) & \"|\" & (duration of current track as text) & \"|\" & "
              "(name of current track) & \"|\" & (artist of current track) & \"|\" & "
              "(album of current track)' 2>/dev/null";

    std::string answer;
    if ( !rc_RunHidden( script, answer ) )
        return false;

    Trim( answer );
    if ( answer.empty() )
        return false;

    std::vector< std::string > parts = Split( answer, '|' );
    if ( parts.size() < 6 )
        return false;

    out.playing  = ( parts[0] == "playing" );
    out.position = (float)atof( parts[1].c_str() );
    // Music reports the track length in seconds, Spotify in milliseconds
    out.length   = (float)atof( parts[2].c_str() );
    if ( out.length > 10000.0f )
        out.length /= 1000.0f;
    out.title    = parts[3];
    out.artist   = parts[4];
    out.album    = parts[5];

    return !out.title.empty();
}

bool AskSystem( rc_Playing & out )
{
    return AskPlayer( "Spotify", out ) || AskPlayer( "Music", out );
}

#elif defined( _WIN32 ) || defined( WIN32 )

//! Only the player's own window counts. A browser puts the page title into a
//! window of the same class, so a video left paused in a tab would otherwise
//! stand in for the music.
bool BelongsToSpotify( HWND window )
{
    DWORD pid = 0;
    GetWindowThreadProcessId( window, &pid );
    if ( !pid )
        return false;

    HANDLE process = OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid );
    if ( !process )
        return false;

    char path[ MAX_PATH ] = { 0 };
    DWORD room = sizeof( path );
    bool told = QueryFullProcessImageNameA( process, 0, path, &room ) != 0;
    CloseHandle( process );

    if ( !told )
        return false;

    char const * name = strrchr( path, '\\' );
    name = name ? name + 1 : path;

    return _stricmp( name, "Spotify.exe" ) == 0;
}

struct Hunt
{
    bool        running;    //!< the player is up, whatever it is doing
    std::string found;      //!< the track, when the title carries one

    Hunt() : running( false ) {}
};

BOOL CALLBACK LookAtWindow( HWND window, LPARAM through )
{
    if ( !IsWindowVisible( window ) || !BelongsToSpotify( window ) )
        return TRUE;

    Hunt * hunt = (Hunt *)through;
    hunt->running = true;

    char title[ 512 ] = { 0 };
    if ( GetWindowTextA( window, title, sizeof( title ) - 1 ) < 3 )
        return TRUE;

    std::string text = title;

    // Paused or idle, the window is named after the product rather than a
    // track. That is not silence to be ignored: it is how this player says it
    // stopped, and treating it as no answer left the display running on.
    if ( text == "Spotify" || text == "Spotify Free" || text == "Spotify Premium" )
        return TRUE;

    // A track reads "Artist - Title"; anything else is some window of its own.
    if ( text.find( " - " ) == std::string::npos )
        return TRUE;

    hunt->found = text;
    return FALSE;
}

//! Reads what the player put in its own title bar. It carries no position and
//! no length, so there is nothing here for a progress bar; it is what a
//! machine gets when the companion could not be started.
bool AskFromTitleBar( rc_Playing & out )
{
    Hunt hunt;
    EnumWindows( LookAtWindow, (LPARAM)&hunt );

    if ( !hunt.running )
        return false;

    // Held on to so that a pause can be shown as the track that is paused,
    // rather than as nothing at all.
    static std::string lastArtist, lastTitle;

    if ( hunt.found.empty() )
    {
        out.artist  = lastArtist;
        out.title   = lastTitle;
        out.playing = false;
    }
    else
    {
        size_t split = hunt.found.find( " - " );
        lastArtist = hunt.found.substr( 0, split );
        lastTitle  = hunt.found.substr( split + 3 );

        out.artist  = lastArtist;
        out.title   = lastTitle;
        out.playing = true;
    }

    out.album    = "";
    out.position = 0.0f;
    out.length   = 0.0f;

    return true;
}

//! What is left when the bridge is not running. The session the system keeps
//! is where a position and a length would come from, but reaching it from
//! inside the game is what took the game down, so that is the bridge's job
//! and this is only the name of the track.
bool AskSystem( rc_Playing & out )
{
    return AskFromTitleBar( out );
}

#else

//! One question, put to one player. An empty name lets the tool choose.
//! @return false when nothing came back worth showing.
bool Ask( char const * player, rc_Playing & out )
{
    std::string who;
    if ( player && *player )
        who = std::string( " --player=" ) + player;

    // stderr is kept: the tool says "No player ..." there and says nothing at
    // all on stdout, so throwing it away turns silence into false success.
    std::string answer;
    rc_RunHidden( "playerctl" + who + " metadata --format "
                  "'{{playerName}}|{{status}}|{{position}}|{{mpris:length}}"
                  "|{{title}}|{{artist}}|{{album}}' 2>&1", answer );
    Trim( answer );


    std::vector< std::string > parts = Split( answer, '|' );
    if ( parts.size() >= 5 && !parts[4].empty() )
    {
        out.who      = parts[0];
        out.playing  = ( parts[1] == "Playing" );
        out.position = (float)( atof( parts[2].c_str() ) / 1000000.0 );  // microseconds
        out.length   = (float)( atof( parts[3].c_str() ) / 1000000.0 );
        out.title    = parts[4];
        out.artist   = parts.size() > 5 ? parts[5] : "";
        out.album    = parts.size() > 6 ? parts[6] : "";
        return true;
    }

    // Asking for all of it in one breath comes back completely empty from a
    // player that is missing any one of the fields, and a position is exactly
    // the field such players tend to lack. So each is asked for on its own,
    // and the track is shown without a progress bar rather than not at all.
    std::string title;
    if ( !Field( who, "metadata title", title ) )
        return false;

    std::string value;
    out.title = title;
    out.who   = Field( who, "status --format '{{playerName}}'", value ) ? value : std::string( player ? player : "" );
    out.playing  = Field( who, "status", value ) && value == "Playing";
    out.artist   = Field( who, "metadata artist", value ) ? value : "";
    out.album    = Field( who, "metadata album", value ) ? value : "";
    out.position = 0.0f;
    out.length   = 0.0f;

    return true;
}

//! MPRIS, through the tool most desktops already have for it.
bool AskSystem( rc_Playing & out )
{
    // Music is asked for by name first. Naming players is not a list to try in
    // turn: the tool takes the first one on it that exists and stops there. So
    // a browser with a video open cannot win over the music that is meant to
    // be on screen, while a machine with no music player still falls through
    // to the second question, which lands on whatever else is playing.
    bool got = Ask( "spotify,spotifyd", out ) || Ask( "", out );


    return got;
}

#endif

}

bool rc_TellPlayer( int what )
{
#if defined( __APPLE__ )
    char const * verb = ( what < 0 ) ? "previous track"
                      : ( what > 0 ) ? "next track"
                                     : "playpause";

    for ( int i = 0; i < 2; ++i )
    {
        char const * app = i == 0 ? "Spotify" : "Music";
        std::string script = "osascript -e 'if application \"";
        script += app;
        script += "\" is running then tell application \"";
        script += app;
        script += "\" to ";
        script += verb;
        script += "' 2>/dev/null";

        std::string ignored;
        if ( rc_RunHidden( script, ignored ) )
            return true;
    }
    return false;

#elif defined( _WIN32 ) || defined( WIN32 )
    // the transport keys, which every player listens for
    BYTE key;
    switch ( what )
    {
    case -1: key = VK_MEDIA_PREV_TRACK; break;
    case  1: key = VK_MEDIA_NEXT_TRACK; break;
    default: key = VK_MEDIA_PLAY_PAUSE; break;
    }
    keybd_event( key, 0, KEYEVENTF_EXTENDEDKEY, 0 );
    keybd_event( key, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0 );
    return true;

#else
    char const * verb = ( what < 0 ) ? "previous"
                      : ( what > 0 ) ? "next"
                                     : "play-pause";

    // The player that answered last is the one whose track is on screen, so it
    // is the one told what to do. Left unnamed, a button under a browser's
    // song would just as happily pause music playing somewhere else.
    if ( lastGood && !lastAnswer.who.empty() &&
         Tell( " --player=" + lastAnswer.who, verb ) )
        return true;

    return Tell( "", verb );
#endif
}

bool rc_SeekPlayer( float seconds )
{
#if defined( __APPLE__ )
    char script[ 256 ];
    snprintf( script, sizeof( script ),
              "osascript -e 'tell application \"Spotify\" to set player position to %.2f' 2>/dev/null",
              seconds );
    std::string ignored;
    return rc_RunHidden( script, ignored );

#elif defined( _WIN32 ) || defined( WIN32 )
    // the transport keys cannot express a position
    (void)seconds;
    return false;

#else
    char where[ 64 ];
    snprintf( where, sizeof( where ), "position %.2f", seconds );

    if ( lastGood && !lastAnswer.who.empty() &&
         Tell( " --player=" + lastAnswer.who, where ) )
        return true;

    return Tell( "", where );
#endif
}

bool rc_AskNowPlaying( rc_Playing & out )
{
    REAL now = tSysTimeFloat();
    if ( now - lastAsked < askEvery )
    {
        if ( lastGood )
            out = lastAnswer;
        return lastGood;
    }

    lastAsked = now;

    rc_Playing fresh;
    lastGood = AskSystem( fresh );

    if ( lastGood )
    {
        lastAnswer = fresh;
        out = fresh;
    }

    return lastGood;
}
