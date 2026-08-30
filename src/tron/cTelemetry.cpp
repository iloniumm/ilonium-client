#include "cTelemetry.h"
#include "cBackend.h"
#include "cIlonium.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace
{
//! Which system this is, at the coarsest useful grain.
//!
//! Not the distribution, not the release, not the machine - a build of the
//! client either runs on Windows or it does not, and that is the whole of what
//! a count of players needs. The detailed picture belongs to a ticket, where
//! somebody has asked for help and knows what they are sending.
char const * WhichSystem()
{
#if defined( _WIN32 ) || defined( WIN32 )
    return "Windows";
#elif defined( __APPLE__ )
    return "macOS";
#elif defined( __linux__ )
    return "Linux";
#else
    return "";
#endif
}

//! Reads one string out of a flat json object.
//!
//! The answer to a hello has three fields in it and no nesting. A parser would
//! be a dependency taken on for eight lines of work.
std::string Field( std::string const & json, char const * name )
{
    std::string const key = std::string( "\"" ) + name + "\"";

    size_t at = json.find( key );
    if ( at == std::string::npos )
        return std::string();

    at = json.find( ':', at + key.size() );
    if ( at == std::string::npos )
        return std::string();

    size_t const from = json.find( '"', at );
    if ( from == std::string::npos )
        return std::string();

    size_t const to = json.find( '"', from + 1 );
    if ( to == std::string::npos )
        return std::string();

    return json.substr( from + 1, to - from - 1 );
}

//! Which of two versions is the later one.
/**
 *  Returns true when \a other is newer than \a mine.
 *
 *  A version is a run of numbers and, on a beta, a word and a number after
 *  them. The numbers decide first, and when they are equal a build with no
 *  suffix wins: 1.3.0 is the finished thing that 1.3.0-beta5 was leading up to.
 *  Two betas of the same version are told apart by the number on the end.
 *
 *  Comparing the two as text and calling any difference an update is how a
 *  client running ahead of the server was told to go and install something
 *  older than itself.
 */
bool Newer( std::string const & mine, std::string const & other )
{
    auto split = []( std::string const & v, std::vector< int > & out, int & tail )
    {
        tail = -1;                       // no suffix at all
        size_t const dash = v.find( '-' );
        std::string const head = v.substr( 0, dash );

        size_t at = 0;
        while ( at < head.size() )
        {
            size_t const dot = head.find( '.', at );
            out.push_back( atoi( head.substr( at, dot - at ).c_str() ) );
            if ( dot == std::string::npos ) break;
            at = dot + 1;
        }

        if ( dash == std::string::npos )
            return;

        // the number on the end of the word, if there is one
        std::string const suffix = v.substr( dash + 1 );
        size_t digits = suffix.size();
        while ( digits > 0 && isdigit( (unsigned char)suffix[ digits - 1 ] ) )
            --digits;

        tail = digits < suffix.size() ? atoi( suffix.c_str() + digits ) : 0;
    };

    std::vector< int > a, b;
    int aTail = -1, bTail = -1;
    split( mine, a, aTail );
    split( other, b, bTail );

    for ( size_t i = 0; i < a.size() || i < b.size(); ++i )
    {
        int const one = i < a.size() ? a[i] : 0;
        int const two = i < b.size() ? b[i] : 0;
        if ( one != two )
            return two > one;
    }

    if ( aTail < 0 || bTail < 0 )
        return aTail >= 0 && bTail < 0;   // ours is a beta, theirs is not

    return bTail > aTail;
}

std::mutex  s_lock;
std::string s_latest;
std::string s_url;
std::string s_notes;
}

bool cTelemetry::s_HeartbeatStarted = false;

void cTelemetry::Init()
{
    // The key the whole backend knows us by. Made on first run, kept beside the
    // configuration, and the same one the chat and the tickets sign with.
    rc_IloniumReady();
}

std::string cTelemetry::GetClientId()
{
    return rc_IloniumPublicKey();
}

void cTelemetry::Say()
{
    std::string body = "{\"version\":\"";
    body += RC_CLIENT_VERSION;
    body += "\",\"os\":\"";
    body += WhichSystem();
    body += "\"}";

    long code = 0;
    std::string answer;
    if ( !rc_IloniumCall( "POST", "/v1/ping", body, code, answer ) || code != 200 )
        return;

    // The same answer carries what the current release is. Read here rather
    // than asked for separately: one request, and nobody has to remember to go
    // and look at a download page.
    std::string const latest = Field( answer, "latest" );
    if ( latest.empty() )
        return;

    std::lock_guard< std::mutex > hold( s_lock );
    s_latest = latest;
    s_url    = Field( answer, "url" );
    s_notes  = Field( answer, "notes" );
}

bool rc_UpdateWaiting()
{
    std::lock_guard< std::mutex > hold( s_lock );
    return !s_latest.empty() && Newer( RC_CLIENT_VERSION, s_latest );
}

std::string rc_UpdateVersion()
{
    std::lock_guard< std::mutex > hold( s_lock );
    return s_latest;
}

std::string rc_UpdateNotes()
{
    std::lock_guard< std::mutex > hold( s_lock );
    return s_notes;
}

std::string rc_UpdateUrl()
{
    std::lock_guard< std::mutex > hold( s_lock );
    return s_url.empty() ? std::string( "https://ilonium.dev/download.html" ) : s_url;
}

void cTelemetry::StartHeartbeat()
{
    if ( s_HeartbeatStarted )
        return;
    s_HeartbeatStarted = true;

    // Says hello, and says nothing else.
    //
    // This used to carry a secret copied out of the binary and an identifier
    // the client made up, which is another way of saying anybody could send it
    // and anybody could send a thousand of them. Now the request is signed with
    // this installation's own key, the way every other request to the backend
    // is, and it carries no numbers at all - how many people are playing is
    // counted at the far end, from the arrivals it could verify.
    std::thread( []()
    {
        for ( ;; )
        {
            Say();
            std::this_thread::sleep_for( std::chrono::minutes( 5 ) );
        }
    } ).detach();
}
