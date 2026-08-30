#include "gLogins.h"

#include "rConsole.h"
#include "nServerInfo.h"

// Where we are connected. Declared out here on purpose: inside a namespace an
// extern names a symbol of that namespace, which links to nothing.
extern tString sg_lastServerIP;
extern unsigned int sg_lastServerPort;
extern bool sg_hasLastServer;
#include "tSysTime.h"
#include "tString.h"
#include "tLocale.h"
#include "ePlayer.h"
#include "gStats.h"

#include <map>
#include <vector>
#include <string>
#include <string.h>

namespace
{

std::map< std::string, std::string > s_logins;
int s_read = 0;

//! The same text without its colour codes.
//!
//! Names arrive painted - eight characters of 0xRRGGBB in front of every
//! change - and the announcement and the player list do not always carry the
//! same ones. Comparing what is left is the only way they match.
std::string Plain( std::string const & in )
{
    std::string out;
    out.reserve( in.size() );

    for ( size_t i = 0; i < in.size(); )
    {
        if ( in[i] == '0' && i + 1 < in.size() && in[i+1] == 'x' && i + 8 <= in.size() )
        {
            i += 8;
            continue;
        }

        out += in[i];
        ++i;
    }

    // and without the space it tends to be padded with
    size_t a = out.find_first_not_of( " \t\r\n" );
    if ( a == std::string::npos )
        return std::string();

    size_t b = out.find_last_not_of( " \t\r\n" );
    return out.substr( a, b - a + 1 );
}

//! The name in an announcement, which may be preceded by an order clause.
std::string Speaker( std::string const & before )
{
    // "Order of admin: somebody" - the name is what follows the colon
    size_t colon = before.rfind( ": " );
    if ( colon != std::string::npos )
        return Plain( before.substr( colon + 2 ) );

    return Plain( before );
}

void Note( std::string const & line )
{
    static char const * const kIn = " has been logged in as ";
    static char const * const kOut = " has been logged out as ";

    size_t at = line.find( kIn );
    if ( at != std::string::npos )
    {
        std::string who = Speaker( line.substr( 0, at ) );
        std::string rest = line.substr( at + strlen( kIn ) );

        // the plain form ends at the full stop, the other at the access level
        size_t stop = rest.find( " at access level" );
        if ( stop == std::string::npos )
            stop = rest.rfind( '.' );
        if ( stop != std::string::npos )
            rest = rest.substr( 0, stop );

        std::string as = Plain( rest );
        if ( !who.empty() && !as.empty() )
            s_logins[ who ] = as;

        return;
    }

    at = line.find( kOut );
    if ( at != std::string::npos )
    {
        std::string who = Speaker( line.substr( 0, at ) );
        if ( !who.empty() )
            s_logins.erase( who );
    }
}

}

void rc_ScanLogins()
{
#ifndef DEDICATED
    int have = sr_con.GetLineCount();

    // A shorter console means it was cleared or wrapped past what we read;
    // start again rather than index into nothing.
    if ( have < s_read )
        s_read = 0;

    for ( int i = s_read; i < have; ++i )
    {
        tString const & raw = sr_con.GetLine( i );
        Note( std::string( (char const *)raw ) );
    }

    s_read = have;
#endif
}

char const * rc_LoginOf( char const * playerName )
{
    if ( !playerName )
        return 0;

    std::map< std::string, std::string >::const_iterator found =
        s_logins.find( Plain( std::string( playerName ) ) );

    return found == s_logins.end() ? 0 : found->second.c_str();
}

void rc_ForgetLogins()
{
    s_logins.clear();
    s_read = 0;
}

// ---------------------------------------------------------------------------

// Who the server itself says is logged in.
//
// The console announcements below only cover what was said while we were
// watching: somebody who logged in before we arrived is invisible to them.
// The server also publishes the whole picture in its info reply - the list the
// server browser has always shown, names on one side and authenticated names
// on the other, line for line. That is the same reply, asked for again while
// we are playing, so the list in game matches the list in the browser.

namespace
{

//! The info record for the server we are on, if we have one.
nServerInfo * OurServer()
{
    if ( !sg_hasLastServer )
        return 0;

    for ( nServerInfo * s = nServerInfo::GetFirstServer(); s; s = s->Next() )
    {
        if ( s->GetPort() != sg_lastServerPort )
            continue;

        if ( s->GetConnectionName() != sg_lastServerIP )
            continue;

        return s;
    }

    return 0;
}

//! Splits a run of lines.
void Lines( tString const & from, std::vector< std::string > & out )
{
    std::string one;

    for ( int i = 0; i < from.Len() - 1; ++i )
    {
        char const c = from[i];

        if ( c == '\n' )
        {
            out.push_back( one );
            one.clear();
            continue;
        }

        one += c;
    }

    if ( !one.empty() )
        out.push_back( one );
}

}

void rc_LoginsFromServer()
{
#ifndef DEDICATED
    if ( sn_GetNetState() != nCLIENT )
        return;

    nServerInfo * ours = OurServer();
    if ( !ours )
        return;

    // Asked again now and then: the reply we have is from whenever the browser
    // last spoke to it, and people log in during a match.
    static double asked = 0;
    double const now = tSysTimeFloat();
    if ( now - asked > 30.0 )
    {
        asked = now;
        ours->QueryServer();
    }

    std::vector< std::string > names, ids;
    Lines( ours->UserNames(), names );
    Lines( ours->UserGlobalIDs(), ids );

    for ( size_t i = 0; i < names.size() && i < ids.size(); ++i )
    {
        std::string const who = Plain( names[i] );
        std::string const as = Plain( ids[i] );

        if ( who.empty() || as.empty() )
            continue;

        s_logins[ who ] = as;
    }
#endif
}
