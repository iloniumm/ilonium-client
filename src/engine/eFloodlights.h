// Lamps around the arena, and the light they put on the floor.
//
// The grid has always been lit by nothing: the floor glows because it is drawn
// glowing, and there is no source anywhere for any of it. Standing lamps give
// the place somewhere for the light to come from - a mast at the rim, a hot
// head at the top of it, a halo around that, and a pool of light thrown across
// the ground in front.
//
// None of it is lighting in the technical sense; there is no shading model
// underneath and the cycles do not cast shadows. It is the older trick, the
// one arcade cabinets used: draw the light itself, additively, and let the eye
// do the rest. Which is also why it costs almost nothing - a few dozen
// triangles for the whole ring.
//
// Where the lamps stand is read from the arena rather than assumed. Walking
// the rim and spacing them along its length means a square arena, an octagon
// and somebody's hand-drawn map all get a sensible ring without anybody
// configuring it.

#ifndef RC_EFLOODLIGHTS_H
#define RC_EFLOODLIGHTS_H

#include "defs.h"
#include "eCoord.h"

class eCamera;

//! The settings, for whatever wants to put them on screen. They are ordinary
//! locals - nothing here is sent over the network, so a lamp is yours alone.
extern bool sg_floodlights;
extern int  sg_floodlightMode;      //!< 0 whole floor, 1 the zones, 2 one per base
extern int  sg_floodlightCount;
extern REAL sg_floodlightHeight;
extern REAL sg_floodlightReach;
extern REAL sg_floodlightBright;
extern bool sg_floodlightShafts;
extern REAL sg_floodR, sg_floodG, sg_floodB;

//! Whether the lamps are switched on at all.
bool rc_Floodlights();

//! The pools of light on the ground.
//!
//! Drawn with the floor, before anything that stands on it, so a cycle passing
//! through a pool is lit by it rather than painted over.
void rc_FloodlightPools();

//! The masts, their heads, the halos and the shafts of light.
//!
//! Drawn after the walls: these are objects in the arena and want to be
//! occluded by everything nearer than they are.
void rc_FloodlightFixtures();

//! Where the lights should point, when the game has somewhere in mind.
//!
//! Zones are the game's business, not the engine's, and the link order puts
//! this side first - so the arena tells the lights where its zones are rather
//! than the lights going to look. Called once when a round is built; an empty
//! list means there is nothing to aim at and the masts fall back to lighting
//! the whole floor.
void rc_FloodlightTargets( eCoord const * points, REAL const * radii, int count );

//! Forgets where the lamps were standing.
//!
//! They are placed from the rim, and the rim is rebuilt with every round on a
//! new map - so the ring has to be worked out again rather than left pointing
//! at an arena that no longer exists.
void rc_FloodlightsForget();

#endif
