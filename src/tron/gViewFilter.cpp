#include "gViewFilter.h"
#include "gCycle.h"
#include "ePlayer.h"
#include "tConfiguration.h"

bool sg_hideOtherCycles = false;
bool sg_hideOtherTrails = false;

static tConfItem<bool> conf_hideCycles( "MOD_HIDE_OTHER_CYCLES", sg_hideOtherCycles );
static tConfItem<bool> conf_hideTrails( "MOD_HIDE_OTHER_TRAILS", sg_hideOtherTrails );

bool rc_HideOtherCycles()
{
    return sg_hideOtherCycles;
}

bool rc_HideOtherTrails()
{
    return sg_hideOtherTrails;
}

bool rc_IsMine( gCycle const * cycle )
{
    if ( !cycle )
        return false;

    ePlayerNetID const * who = cycle->Player();
    if ( !who )
        return false;

    // Every seat at this machine counts, not only the first: a split screen
    // has two riders and both of them need to see themselves.
    for ( int i = 0; i < MAX_PLAYERS; ++i )
    {
        ePlayer * seat = ePlayer::PlayerConfig( i );
        if ( seat && seat->netPlayer == who )
            return true;
    }

    return false;
}
