// What this client has seen you do.
//
// Everything here is counted locally and belongs to whoever is sitting at the
// machine. It starts at nothing on a fresh install and only ever grows from
// what actually happens - there is no seeding it with a plausible looking
// history, because a number somebody knows is invented is worth less than no
// number at all.

#ifndef RC_GSTATS_H
#define RC_GSTATS_H

#include "defs.h"

#include <string>
#include <vector>

struct rcDay
{
    std::string when;      //!< YYYY-MM-DD
    double      seconds;   //!< time in a match that day
    int         rounds;
    int         kills;
    int         deaths;

    rcDay() : seconds( 0 ), rounds( 0 ), kills( 0 ), deaths( 0 ) {}
};

struct rcStats
{
    std::string firstSeen;

    double clientSeconds;  //!< with the client open
    double matchSeconds;   //!< actually riding
    double distance;       //!< travelled, in the game's own units
    double bestLife;       //!< longest single life

    int rounds;
    int deaths;
    int kills;
    int sessions;

    std::vector< rcDay > days;   //!< most recent last

    rcStats()
        : clientSeconds( 0 ), matchSeconds( 0 ), distance( 0 ), bestLife( 0 )
        , rounds( 0 ), deaths( 0 ), kills( 0 ), sessions( 0 ) {}
};

//! Everything counted so far.
rcStats const & rc_Stats();

//! Called once a frame. Works out on its own whether anything is happening.
void rc_StatsTick( REAL dt );

//! Read at start up, written as it goes and on the way out.
void rc_StatsLoad();
void rc_StatsSave();

//! Called whenever a cycle dies, on the one path both a local game and a
//! server go through. Works out for itself whether this machine did the
//! killing, the dying, or neither.
class ePlayerNetID;
void rc_StatsDeath( ePlayerNetID const * prey, ePlayerNetID const * hunter );

//! How many days in a row up to today. A streak is the one number in here
//! that changes how somebody feels about opening the client tomorrow.
int rc_StatsStreak();

//! Seconds this run, for the session line.
double rc_StatsSession();

#endif
