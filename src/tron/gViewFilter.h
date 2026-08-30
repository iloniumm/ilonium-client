// What this client chooses not to draw.
//
// None of this changes the game. A wall that is not drawn still kills, a rider
// who is not drawn still takes up the same space, and anybody turning these on
// is making things harder for themselves rather than easier. They exist for
// the two honest reasons people ask for them: a screen with fewer things on it
// on a machine that struggles, and a clearer view of one's own line.
//
// Only ever other people. A rider who cannot see their own machine has no way
// to play at all, so that case is refused rather than left to a setting.

#ifndef RC_GVIEWFILTER_H
#define RC_GVIEWFILTER_H

class gCycle;

//! True while other people's machines are hidden, name and all.
bool rc_HideOtherCycles();

//! True while other people's walls are hidden.
bool rc_HideOtherTrails();

//! Whether this is the machine the person at this keyboard is riding.
bool rc_IsMine( gCycle const * cycle );

#endif
