#include "gStats.h"
#include "gCycle.h"
#include "ePlayer.h"
#include "tDirectories.h"
#include "tSysTime.h"
#include "ModTheme.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

namespace
{

rcStats kept;
bool    loaded = false;

double sessionSeconds = 0.0;
double lifeSeconds = 0.0;
double sinceSaved = 0.0;

bool wasRiding = false;

//! Today, the way the file writes it.
std::string Today()
{
    time_t now = time( NULL );
    struct tm * local = localtime( &now );

    char text[ 16 ] = "0000-00-00";
    if ( local )
        strftime( text, sizeof( text ), "%Y-%m-%d", local );

    return std::string( text );
}

//! The bucket for today, made if this is the first time it has been asked for.
//! Twenty weeks are kept, which is what the calendar below can show. Past that
//! it is history nobody looks at and the file grows for no reason.
rcDay & Bucket()
{
    std::string when = Today();

    if ( kept.days.empty() || kept.days.back().when != when )
    {
        rcDay fresh;
        fresh.when = when;
        fresh.seconds = 0.0;
        kept.days.push_back( fresh );

        while ( kept.days.size() > 140 )
            kept.days.erase( kept.days.begin() );
    }

    return kept.days.back();
}

//! The cycle the person at this machine is riding, or nothing.
gCycle * Mine()
{
    ePlayer * seat = ePlayer::PlayerConfig( 0 );
    if ( !seat || !seat->netPlayer )
        return NULL;

    eGameObject * body = seat->netPlayer->Object();
    if ( !body )
        return NULL;

    return dynamic_cast< gCycle * >( body );
}

std::string Where()
{
    return std::string( (char const *)rc_InternalPath( "rc_stats.txt" ) );
}

}

int rc_StatsStreak()
{
    if ( kept.days.empty() )
        return 0;

    // Counted back from today, and yesterday counts as still going: somebody
    // who has not played yet this morning has not lost the run.
    time_t now = time( NULL );
    int streak = 0;

    for ( int back = 0; back < 400; ++back )
    {
        time_t stamp = now - (time_t)back * 24 * 3600;
        struct tm when = *localtime( &stamp );

        char date[ 16 ];
        strftime( date, sizeof( date ), "%Y-%m-%d", &when );

        bool played = false;
        for ( size_t i = 0; i < kept.days.size(); ++i )
            if ( kept.days[i].when == date && kept.days[i].seconds > 30.0 )
            {
                played = true;
                break;
            }

        if ( played )
            ++streak;
        else if ( back > 0 )
            break;
    }

    return streak;
}

rcStats const & rc_Stats()
{
    return kept;
}

double rc_StatsSession()
{
    return sessionSeconds;
}

void rc_StatsLoad()
{
    if ( loaded )
        return;
    loaded = true;

    FILE * f = fopen( Where().c_str(), "r" );

    if ( !f )
    {
        // Nothing yet, and that is the honest starting point.
        kept.firstSeen = Today();
        kept.sessions = 1;
        return;
    }

    char line[ 256 ];
    while ( fgets( line, sizeof( line ), f ) )
    {
        char key[ 64 ] = { 0 };
        char rest[ 190 ] = { 0 };

        if ( sscanf( line, "%63s %189[^\n]", key, rest ) != 2 )
            continue;

        if ( !strcmp( key, "first" ) )        kept.firstSeen = rest;
        else if ( !strcmp( key, "client" ) )  kept.clientSeconds = atof( rest );
        else if ( !strcmp( key, "match" ) )   kept.matchSeconds = atof( rest );
        else if ( !strcmp( key, "distance" ) )kept.distance = atof( rest );
        else if ( !strcmp( key, "bestlife" ) )kept.bestLife = atof( rest );
        else if ( !strcmp( key, "rounds" ) )  kept.rounds = atoi( rest );
        else if ( !strcmp( key, "deaths" ) )  kept.deaths = atoi( rest );
        else if ( !strcmp( key, "kills" ) )   kept.kills = atoi( rest );
        else if ( !strcmp( key, "sessions" ) )kept.sessions = atoi( rest );
        else if ( !strcmp( key, "day" ) )
        {
            char when[ 16 ] = { 0 };
            double howLong = 0.0;
            int r = 0, k = 0, d = 0;

            // The first version of this file only had a date and a length.
            // Read either, so nobody's history is thrown away for the sake of
            // three more columns.
            int got = sscanf( rest, "%15s %lf %d %d %d", when, &howLong, &r, &k, &d );
            if ( got >= 2 )
            {
                rcDay day;
                day.when = when;
                day.seconds = howLong;
                day.rounds = r;
                day.kills = k;
                day.deaths = d;
                kept.days.push_back( day );
            }
        }
    }

    fclose( f );

    if ( kept.firstSeen.empty() )
        kept.firstSeen = Today();

    ++kept.sessions;
}

void rc_StatsSave()
{
    if ( !loaded )
        return;

    // Written aside and moved into place: this is updated while the game runs,
    // and a half written file would lose everything rather than one minute.
    std::string path = Where();
    std::string temp = path + ".new";

    FILE * f = fopen( temp.c_str(), "w" );
    if ( !f )
        return;

    fprintf( f, "# retrocycles, counted locally\n" );
    fprintf( f, "first %s\n", kept.firstSeen.c_str() );
    fprintf( f, "client %.1f\n", kept.clientSeconds );
    fprintf( f, "match %.1f\n", kept.matchSeconds );
    fprintf( f, "distance %.1f\n", kept.distance );
    fprintf( f, "bestlife %.1f\n", kept.bestLife );
    fprintf( f, "rounds %d\n", kept.rounds );
    fprintf( f, "deaths %d\n", kept.deaths );
    fprintf( f, "kills %d\n", kept.kills );
    fprintf( f, "sessions %d\n", kept.sessions );

    for ( size_t i = 0; i < kept.days.size(); ++i )
        fprintf( f, "day %s %.1f %d %d %d\n", kept.days[i].when.c_str(), kept.days[i].seconds,
                 kept.days[i].rounds, kept.days[i].kills, kept.days[i].deaths );

    fclose( f );

    remove( path.c_str() );
    rename( temp.c_str(), path.c_str() );
}

void rc_StatsDeath( ePlayerNetID const * prey, ePlayerNetID const * hunter )
{
    if ( !loaded )
        return;

    ePlayer * seat = ePlayer::PlayerConfig( 0 );
    if ( !seat )
        return;

    ePlayerNetID const * ours = seat->netPlayer;
    if ( !ours )
        return;

    if ( prey == ours )
    {
        ++kept.deaths;
        ++Bucket().deaths;
    }

    if ( hunter != ours || prey == ours )
        return;

    // A frag is somebody on another team, taken out by us. A suicide is not an
    // achievement and a team kill is the opposite of one; the game does not
    // score either, and neither does this.
    if ( prey && ours->CurrentTeam() && prey->CurrentTeam() == ours->CurrentTeam() )
        return;

    ++kept.kills;
    ++Bucket().kills;

    if ( kept.kills > 0 && kept.kills % 100 == 0 )
    {
        char said[ 64 ];
        snprintf( said, sizeof( said ), "%d taken out", kept.kills );
        rc_Toast( said, "confirm" );
    }
}

void rc_StatsTick( REAL )
{
    if ( !loaded )
        rc_StatsLoad();

    // Timed from the clock rather than from whatever was passed in. Several
    // screens draw themselves on their own frame, and each of them calls this;
    // counted that way an hour in the menu would be recorded as three.
    static REAL last = 0.0f;
    REAL now = tSysTimeFloat();

    if ( last <= 0.0f )
    {
        last = now;
        return;
    }

    REAL dt = now - last;
    last = now;

    // A frame that took a second and a half means the machine was busy
    // elsewhere - loading, or dragged into the background. Counting it as time
    // played would be generous in the wrong direction.
    if ( dt <= 0.0f || dt > 1.5f )
        return;

    kept.clientSeconds += dt;
    sessionSeconds += dt;

    gCycle * mine = Mine();
    bool riding = mine && mine->Alive();

    if ( riding )
    {
        kept.matchSeconds += dt;
        lifeSeconds += dt;
        Bucket().seconds += dt;

        kept.distance += mine->Speed() * dt;

        // A round is counted when riding begins, which is also the only
        // moment this can know about without asking the game for its own
        // notion of one.
        if ( !wasRiding )
        {
            ++kept.rounds;
            ++Bucket().rounds;

            // Round numbers, rarely enough that they still mean something.
            if ( kept.rounds > 0 && kept.rounds % 250 == 0 )
            {
                char said[ 64 ];
                snprintf( said, sizeof( said ), "%d rounds ridden", kept.rounds );
                rc_Toast( said, "confirm" );
            }
        }
    }
    else if ( wasRiding )
    {
        // Stopped riding. Whether that was a wall or the round ending, the
        // life is over either way, and the longest one is worth keeping.
        if ( lifeSeconds > kept.bestLife )
        {
            // Only worth saying out loud once there is something to beat.
            // Congratulating somebody on their first ever life is hollow.
            bool worth = kept.bestLife > 5.0 && lifeSeconds > kept.bestLife + 0.5;

            kept.bestLife = lifeSeconds;

            if ( worth )
            {
                char said[ 64 ];
                int all = (int)lifeSeconds;
                snprintf( said, sizeof( said ), "longest life yet - %dm %02ds", all / 60, all % 60 );
                rc_Toast( said, "confirm" );
            }
        }

        // The life is over, but not every life that ends is a death: a round
        // that runs out leaves everybody standing. Deaths are counted where
        // the game says somebody died and nowhere else.
        lifeSeconds = 0.0;
    }

    wasRiding = riding;

    sinceSaved += dt;
    if ( sinceSaved > 20.0 )
    {
        sinceSaved = 0.0;
        rc_StatsSave();
    }
}
