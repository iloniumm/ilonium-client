#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef DEDICATED

#include "cDiscord.h"

#include "tConfiguration.h"
#include "tSysTime.h"
#include "tDirectories.h"
#include "ePlayer.h"
#include "eTeam.h"
#include "eTimer.h"
#include "nNetwork.h"

#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
// windows.h rewrites a long list of names to their A or W variant, and
// GetUserName is one of them - which turns a perfectly good call on a player
// into a call to a method nobody declared. The engine's headers are above
// this line, so their declarations keep the name they were written with.
#undef GetUserName
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

bool sg_discord = true;
// The application the status belongs to.
//
// Part of the build rather than a setting: it names this client, the art in
// the status card is uploaded against it, and a copy pointing somewhere else
// would be showing somebody else's name over our game. Public knowledge either
// way - it travels with every status anyone sees.
static char const * const kApplicationId = "1542251018589306941";

// The face of the card. All of it names this client rather than the player,
// so none of it is a setting: the art is uploaded against the application
// above, and a copy with a different picture and the same name would be
// somebody else's client wearing ours.
static char const * const kLargeImage = "logo";
static char const * const kLargeText  = "Ilonium Client";

// A small mark in the corner of the picture, and up to two links under the
// card. The badge waits for art worth putting there; the links go where the
// builds are, one host and its mirror.
//
// Both point at the list of releases rather than at the newest one: a beta is
// published as a prerelease, and the shortcut to "latest" walks straight past
// those and lands on nothing.
static char const * const kBadge     = "";
static char const * const kBadgeText = "";
static char const * const kButtonOne = "Download (GitHub)|https://github.com/iloniumm/ilonium-client/releases";
static char const * const kButtonTwo = "Download (GitLab)|https://gitlab.com/ilonamoer/ilonium-client/-/releases";


static tConfItem<bool> c_discord( "MOD_DISCORD", sg_discord );

extern tString sg_lastServerName;

namespace
{

// ---------------------------------------------------------------------------
// The pipe.
//
// Ten sockets may exist; Discord takes the first free one, so a machine with
// two clients open answers on -1 as well as -0. Where they live depends on how
// Discord was installed, which is why there is a list rather than a path.

#ifdef WIN32
typedef HANDLE Pipe;
static Pipe const kNoPipe = INVALID_HANDLE_VALUE;
#else
typedef int Pipe;
static Pipe const kNoPipe = -1;
#endif

Pipe        s_pipe = kNoPipe;
bool        s_ready = false;        //!< handshake accepted
double      s_nextTry = 0;          //!< when to attempt a connection again
double      s_nextSay = 0;          //!< when the status may change again
std::string s_last;                 //!< what was said, to avoid repeating it
std::string s_note( "off" );        //!< for the line under the setting
double      s_since = 0;            //!< when this match or session began

void Close()
{
    if ( s_pipe != kNoPipe )
    {
#ifdef WIN32
        CloseHandle( s_pipe );
#else
        close( s_pipe );
#endif
        s_pipe = kNoPipe;
    }

    s_ready = false;
    s_last.clear();
}

//! Every place a Discord socket is known to appear, in the order worth trying.
void Candidates( std::vector< std::string > & out, int slot )
{
    char tail[ 24 ];
    snprintf( tail, sizeof( tail ), "discord-ipc-%d", slot );

#ifdef WIN32
    out.push_back( std::string( "\\\\.\\pipe\\" ) + tail );
#else
    char const * roots[] = { getenv( "XDG_RUNTIME_DIR" ), getenv( "TMPDIR" ),
                             getenv( "TMP" ), getenv( "TEMP" ), "/tmp" };

    // Snap and flatpak each put the socket inside their own sandbox
    // directory, so the plain path finds nothing for most Linux installs.
    char const * nests[] = { "", "/snap.discord", "/app/com.discordapp.Discord" };

    for ( size_t r = 0; r < sizeof( roots ) / sizeof( roots[0] ); ++r )
    {
        if ( !roots[r] || !*roots[r] )
            continue;

        for ( size_t n = 0; n < sizeof( nests ) / sizeof( nests[0] ); ++n )
            out.push_back( std::string( roots[r] ) + nests[n] + "/" + tail );
    }
#endif
}

bool Open()
{
    for ( int slot = 0; slot < 10; ++slot )
    {
        std::vector< std::string > paths;
        Candidates( paths, slot );

        for ( size_t i = 0; i < paths.size(); ++i )
        {
#ifdef WIN32
            HANDLE h = CreateFileA( paths[i].c_str(), GENERIC_READ | GENERIC_WRITE,
                                    0, NULL, OPEN_EXISTING, 0, NULL );
            if ( h == INVALID_HANDLE_VALUE )
                continue;

            DWORD mode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
            SetNamedPipeHandleState( h, &mode, NULL, NULL );
            s_pipe = h;
            return true;
#else
            int fd = socket( AF_UNIX, SOCK_STREAM, 0 );
            if ( fd < 0 )
                return false;

            sockaddr_un addr;
            memset( &addr, 0, sizeof( addr ) );
            addr.sun_family = AF_UNIX;

            if ( paths[i].size() + 1 > sizeof( addr.sun_path ) )
            {
                close( fd );
                continue;
            }

            strncpy( addr.sun_path, paths[i].c_str(), sizeof( addr.sun_path ) - 1 );

            if ( connect( fd, (sockaddr *)&addr, sizeof( addr ) ) == 0 )
            {
                // Nothing here may ever wait: a stalled write in the frame
                // loop would be a stutter in the game.
                fcntl( fd, F_SETFL, O_NONBLOCK );
                s_pipe = fd;
                return true;
            }

            close( fd );
#endif
        }
    }

    return false;
}

//! One frame: opcode and length, both four bytes and little endian, then the
//! payload. That is the entire protocol.
bool Send( int opcode, std::string const & payload )
{
    if ( s_pipe == kNoPipe )
        return false;

    std::string frame;
    frame.resize( 8 + payload.size() );

    unsigned int const op = (unsigned int)opcode;
    unsigned int const len = (unsigned int)payload.size();

    for ( int i = 0; i < 4; ++i )
    {
        frame[i]     = (char)( ( op  >> ( i * 8 ) ) & 0xFF );
        frame[4 + i] = (char)( ( len >> ( i * 8 ) ) & 0xFF );
    }

    memcpy( &frame[8], payload.data(), payload.size() );

#ifdef WIN32
    DWORD written = 0;
    if ( !WriteFile( s_pipe, frame.data(), (DWORD)frame.size(), &written, NULL ) )
        return false;
    return written == frame.size();
#else
    size_t sent = 0;
    while ( sent < frame.size() )
    {
        ssize_t n = write( s_pipe, frame.data() + sent, frame.size() - sent );

        if ( n > 0 )
        {
            sent += (size_t)n;
            continue;
        }

        if ( n < 0 && ( errno == EAGAIN || errno == EWOULDBLOCK ) )
            continue;

        return false;
    }

    return true;
#endif
}

//! Reads one frame if Discord has sent one.
//!
//! Only two answers matter. A close frame carries the reason it is closing -
//! an application id that does not exist is the usual one, and saying so is
//! the difference between a setting that looks broken and one that tells you
//! what to fix. Everything else is an acknowledgement.
bool Take( int & opcode, std::string & body )
{
    char head[ 8 ];

#ifdef WIN32
    DWORD got = 0;
    if ( !ReadFile( s_pipe, head, 8, &got, NULL ) || got < 8 )
        return false;
#else
    ssize_t got = read( s_pipe, head, 8 );
    if ( got < 8 )
        return false;
#endif

    unsigned int op = 0, len = 0;
    for ( int i = 0; i < 4; ++i )
    {
        op  |= ( (unsigned char)head[i] )     << ( i * 8 );
        len |= ( (unsigned char)head[4 + i] ) << ( i * 8 );
    }

    if ( len > 8192 )
        return false;

    body.resize( len );

    size_t have = 0;
    while ( have < len )
    {
#ifdef WIN32
        DWORD chunk = 0;
        if ( !ReadFile( s_pipe, &body[have], (DWORD)( len - have ), &chunk, NULL ) || chunk == 0 )
            break;
        have += chunk;
#else
        ssize_t chunk = read( s_pipe, &body[have], len - have );
        if ( chunk > 0 )
        {
            have += (size_t)chunk;
            continue;
        }
        if ( chunk < 0 && ( errno == EAGAIN || errno == EWOULDBLOCK ) )
            continue;
        break;
#endif
    }

    opcode = (int)op;
    return true;
}

//! Pulls the one value out of a small flat answer, without a parser.
std::string Field( std::string const & json, char const * name )
{
    std::string const key = std::string( "\"" ) + name + "\":";
    size_t at = json.find( key );
    if ( at == std::string::npos )
        return std::string();

    at += key.size();
    while ( at < json.size() && ( json[at] == ' ' || json[at] == '"' ) )
        ++at;

    size_t end = at;
    while ( end < json.size() && json[end] != '"' && json[end] != ',' && json[end] != '}' )
        ++end;

    return json.substr( at, end - at );
}

//! Quotes a string the way JSON wants it, and keeps it short enough for the
//! field it is going into.
std::string Quoted( std::string in, size_t most )
{
    if ( in.size() > most )
        in.resize( most );

    std::string out( "\"" );

    for ( size_t i = 0; i < in.size(); ++i )
    {
        unsigned char const c = (unsigned char)in[i];

        switch ( c )
        {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n";  break;
        case '\r': break;
        case '\t': out += "\\t";  break;
        default:
            if ( c < 0x20 )
                break;
            out += (char)c;
            break;
        }
    }

    out += "\"";
    return out;
}

//! Colour codes belong on the arena, not in a status line.
std::string Plain( char const * text )
{
    std::string out;
    if ( !text )
        return out;

    for ( size_t i = 0; text[i]; )
    {
        if ( text[i] == '0' && text[i+1] == 'x' )
        {
            size_t skip = 2;
            while ( skip < 8 && text[i+skip] &&
                    ( isxdigit( (unsigned char)text[i+skip] ) ||
                      text[i+skip] == 'R' || text[i+skip] == 'E' ||
                      text[i+skip] == 'S' || text[i+skip] == 'T' ) )
                ++skip;

            if ( skip == 8 )
            {
                i += skip;
                continue;
            }
        }

        out += text[i];
        ++i;
    }

    return out;
}

// ---------------------------------------------------------------------------
// What to say.

struct Standing
{
    std::string top;      //!< the line in bold: where you are
    std::string bottom;   //!< the line under it: who you are and how you are doing
    int         here;     //!< people on the server
    int         room;     //!< how many it holds
};

//! The name this player goes by, without the colour codes it is stored with.
std::string Whoami()
{
    ePlayer * seat = ePlayer::PlayerConfig( 0 );
    if ( !seat )
        return std::string();

    if ( seat->netPlayer )
    {
        ePlayerNetID * me = seat->netPlayer;
        std::string name = Plain( (char const *)me->GetUserName() );
        if ( !name.empty() )
            return name;
    }

    return Plain( (char const *)seat->Name() );
}

void Look( Standing & out )
{
    out.here = 0;
    out.room = 0;

    bool const online = ( sn_GetNetState() == nCLIENT );

    std::string const me = Whoami();

    if ( !online )
    {
        bool playing = false;
        for ( int i = 0; i < MAX_PLAYERS; ++i )
        {
            ePlayer * seat = ePlayer::PlayerConfig( i );
            if ( seat && seat->netPlayer && seat->netPlayer->CurrentTeam() )
            {
                playing = true;
                break;
            }
        }

        out.top = playing ? "Local game" : "Main menu";
        out.bottom = me;
        return;
    }

    out.here = se_PlayerNetIDs.Len();

    if ( sg_lastServerName.Len() > 1 )
        out.top = Plain( (char const *)sg_lastServerName );
    else
        out.top = "Online";

    ePlayer * seat = ePlayer::PlayerConfig( 0 );
    ePlayerNetID * mine = 0;
    if ( seat )
        mine = seat->netPlayer;

    if ( !mine )
    {
        out.bottom = me + "  -  connecting";
        return;
    }

    if ( mine->IsSpectating() || !mine->CurrentTeam() )
    {
        out.bottom = me + "  -  watching";
        return;
    }

    // Where the player stands, which is the one number worth a glance from
    // outside the game.
    int rank = 1;
    int const points = mine->TotalScore();

    for ( int i = 0; i < se_PlayerNetIDs.Len(); ++i )
    {
        ePlayerNetID * other = se_PlayerNetIDs( i );
        if ( other && other != mine && other->TotalScore() > points )
            ++rank;
    }

    char line[ 160 ];
    snprintf( line, sizeof( line ), "%s  -  %d point%s, %d of %d",
              me.c_str(), points, points == 1 ? "" : "s", rank, se_PlayerNetIDs.Len() );

    out.bottom = line;
}

//! The part that says what is happening. Kept apart from the envelope so it
//! can be compared with what went out last time - the envelope carries a fresh
//! nonce on every call and would never match itself.
std::string Activity()
{
    Standing now;
    Look( now );

    std::string json( "{" );
    json += "\"details\":" + Quoted( now.top, 120 );

    if ( !now.bottom.empty() )
        json += ",\"state\":" + Quoted( now.bottom, 120 );

    // A clock that counts up from when this run began. Discord draws it
    // itself, so it stays right without being told again.
    char stamp[ 48 ];
    snprintf( stamp, sizeof( stamp ), ",\"timestamps\":{\"start\":%lld}", (long long)s_since );
    json += stamp;

    if ( now.here > 0 )
    {
        char party[ 96 ];
        snprintf( party, sizeof( party ), ",\"party\":{\"id\":\"arena\",\"size\":[%d,%d]}",
                  now.here, now.room > now.here ? now.room : now.here );
        json += party;
    }

    if ( *kLargeImage )
    {
        json += ",\"assets\":{\"large_image\":" + Quoted( kLargeImage, 60 );

        if ( *kLargeText )
            json += ",\"large_text\":" + Quoted( kLargeText, 120 );

        if ( *kBadge )
        {
            json += ",\"small_image\":" + Quoted( kBadge, 60 );
            if ( *kBadgeText )
                json += ",\"small_text\":" + Quoted( kBadgeText, 120 );
        }

        json += "}";
    }

    // Up to two buttons, written as "what it says|where it goes". Discord
    // refuses anything that is not a real address and closes the connection
    // over it, so a malformed one is dropped here instead.
    {
        std::string buttons;
        char const * pair[2] = { kButtonOne, kButtonTwo };

        for ( int i = 0; i < 2; ++i )
        {
            std::string const raw( pair[i] );
            size_t const bar = raw.find( '|' );
            if ( bar == std::string::npos || bar == 0 || bar + 1 >= raw.size() )
                continue;

            std::string label = raw.substr( 0, bar );
            std::string url = raw.substr( bar + 1 );

            while ( !label.empty() && label[label.size()-1] == ' ' ) label.erase( label.size()-1 );
            while ( !url.empty() && url[0] == ' ' ) url.erase( 0, 1 );

            if ( url.compare( 0, 7, "http://" ) != 0 && url.compare( 0, 8, "https://" ) != 0 )
                continue;

            if ( label.empty() || label.size() > 31 )
                label.resize( label.empty() ? 0 : 31 );

            if ( label.empty() )
                continue;

            if ( !buttons.empty() )
                buttons += ",";

            buttons += "{\"label\":" + Quoted( label, 31 ) + ",\"url\":" + Quoted( url, 512 ) + "}";
        }

        if ( !buttons.empty() )
            json += ",\"buttons\":[" + buttons + "]";
    }

    json += "}";
    return json;
}

//! Wraps a body in the envelope Discord expects.
std::string Envelope( std::string const & activity )
{
    std::string json( "{\"cmd\":\"SET_ACTIVITY\",\"nonce\":\"" );

    char nonce[ 32 ];
    snprintf( nonce, sizeof( nonce ), "%u", (unsigned int)( tSysTimeFloat() * 1000 ) );
    json += nonce;

    json += "\",\"args\":{\"pid\":";

#ifdef WIN32
    json += std::to_string( (int)GetCurrentProcessId() );
#else
    json += std::to_string( (int)getpid() );
#endif

    json += ",\"activity\":" + activity + "}}";
    return json;
}

}

// ---------------------------------------------------------------------------

bool rc_DiscordConnected()
{
    return s_ready;
}

char const * rc_DiscordStatus()
{
    return s_note.c_str();
}

void rc_DiscordStop()
{
    if ( s_pipe != kNoPipe )
        s_note = "off";

    Close();
    s_nextTry = 0;
    s_nextSay = 0;
}

void rc_DiscordTick()
{
    double const now = tSysTimeFloat();

    if ( !sg_discord )
    {
        if ( s_pipe != kNoPipe )
            rc_DiscordStop();

        return;
    }

    if ( s_since <= 0 )
        s_since = (double)time( 0 );

    // Not connected: try now and then, never every frame.
    if ( s_pipe == kNoPipe )
    {
        if ( now < s_nextTry )
            return;

        s_nextTry = now + 15.0;

        if ( !Open() )
        {
            s_note = "discord is not running";
            return;
        }

        std::string hello( "{\"v\":1,\"client_id\":" );
        hello += Quoted( kApplicationId, 40 );
        hello += "}";

        if ( !Send( 0, hello ) )
        {
            Close();
            s_note = "could not say hello";
            return;
        }

        s_note = "connected";
        s_ready = true;
        s_nextSay = 0;
        return;
    }

    // Discord answers the handshake and every update. A close frame means it
    // will not talk to us at all, and why.
    {
        int opcode = 0;
        std::string body;

        while ( Take( opcode, body ) )
        {
            if ( opcode != 2 )
                continue;

            std::string const why = Field( body, "message" );

            Close();
            s_note = why.empty() ? std::string( "discord closed the pipe" )
                                 : ( "discord says: " + why );
            s_nextTry = now + 30.0;
            return;
        }
    }

    // Discord asks for no more than five updates in twenty seconds. Well
    // inside that, and only when there is something new to say.
    if ( now < s_nextSay )
        return;

    std::string const say = Activity();
    if ( say == s_last )
    {
        s_nextSay = now + 5.0;
        return;
    }

    if ( !Send( 1, Envelope( say ) ) )
    {
        Close();
        s_note = "discord closed the pipe";
        s_nextTry = now + 15.0;
        return;
    }

    s_last = say;
    s_nextSay = now + 5.0;
    s_note = "showing";
}

#endif // DEDICATED
