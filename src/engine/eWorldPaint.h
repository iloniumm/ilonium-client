// The colours of the world the enhanced floor draws.
//
// The floor, the grid over it and the heavier lines every fifth square were
// written into the drawing code as numbers. They are settings now, because the
// one thing a client like this is for is looking the way its owner wants, and
// a colour somebody cannot change is a decision made for them.
//
// Kept out here rather than in the menu so the drawing code can read them
// without knowing anything about menus.

#ifndef RC_EWORLDPAINT_H
#define RC_EWORLDPAINT_H

#include "defs.h"

struct rcPaint
{
    REAL r, g, b;
};

//! The glass under everything.
rcPaint const & rc_FloorPaint();

//! The grid, and the crosses where it meets itself.
rcPaint const & rc_GridPaint();

//! The heavier line every fifth square.
rcPaint const & rc_GridMajorPaint();

#endif

//! How many colours the trail is painted with: one is the plain player colour,
//! two or three blend along its length.
int rc_TrailColourCount();

//! One of the trail's colours, blended by @a along, nought at the far end of
//! the wall and one at the rider.
rcPaint rc_TrailPaint( REAL along );

//! Whether the trail is painted at all, or left as the player's own colour.
bool rc_TrailPainted();

//! The arena's own walls - the ones that hold the grid in - and the same
//! choice of one, two or three colours blended along their length.
bool    rc_RimPainted();
rcPaint rc_RimPaint( REAL along );

//! The outline of a heart, in a box one unit across and centred on nothing in
//! particular - whoever draws it decides where and how big.
//!
//! Kept here because three different things want it now and a curve written
//! out three times is a curve that will be three different shapes by spring.
//! @return how many points there are
int rc_HeartShape( REAL const * & xs, REAL const * & ys );

//! One small solid heart, standing upright in the world.
//!
//! Solid rather than flat: a front face, a back face and a rim joining them,
//! so it has a thickness and catches the light differently as it turns. A flat
//! one is a sticker, and from the side it disappears entirely.
//!
//! Small on purpose, and always the same size regardless of what is throwing
//! it. Scaling these with the force of a blast produced hearts the size of the
//! arena, which is not a nice touch, it is an obstruction.
//!
//! @param size  half the height of the heart, in the game's own units
void rc_HeartSolid( REAL x, REAL y, REAL z, REAL size, REAL turn, REAL alpha );

//! The same heart, for callers drawing a run of them.
//!
//! Opens no batch of its own: the caller opens one, draws as many as it likes
//! and closes it. Each heart is a few dozen triangles, and a batch apiece is
//! what turns a shower of sparks into a frame rate problem.
void rc_HeartSolidInto( REAL x, REAL y, REAL z, REAL size, REAL turn, REAL alpha );

//! Whether a crash throws hearts instead of the usual lines.
bool rc_HeartDeaths();

//! How many hearts a death throws, and how many a scrape throws.
//!
//! The two effects run at wildly different rates - a scrape produces them by
//! the dozen and a death produced a handful - so one number for both would
//! always be wrong at one end.
int rc_HeartDeathCount();
int rc_HeartSparkCount();

//! Hearts drifting up behind your own machine, and how many are in the air.
bool rc_HeartTrail();
int  rc_HeartTrailCount();

//! Whether enough time has passed to throw another shower of sparks.
//!
//! Grinding a wall asks for one on every step, which for a few lines is
//! nothing and for solid shapes is a machine on its knees. This holds them to
//! a rate somebody chose, and says yes the first time it is asked.
bool rc_SparkDue( REAL now );

//! Whether the sparks a collision throws are hearts.
//!
//! Its own setting rather than the one that chooses the shape drifting behind
//! the menu: those are two different things in two different places, and tying
//! them together meant the only way to get hearts in a match was to guess.
bool rc_HeartSparks();

//! The neon floor and the neon walls, asked for on their own.
//!
//! Both arrived as part of one switch that also turns on bloom, shaders and
//! everything else. Somebody who wants the grid without the rest had no way to
//! say so, and somebody whose machine cannot afford the rest was locked out of
//! both. They answer separately now, and the master switch still turns them on
//! the way it always did.
bool rc_NeonFloor();

extern bool sg_floorTexture;
extern REAL sg_floorTextureLight;
extern REAL sg_floorTextureScale;

//! Whether the ground carries its texture under the grid.
bool rc_FloorTexture();
//! How hard that texture is lit, since the floor colour alone leaves it black.
REAL rc_FloorTextureLight();
//! How many grid squares one tile covers.
REAL rc_FloorTextureScale();
//! The colour to draw it in: the floor's hue, at that brightness.
void rc_FloorTextureColour( REAL & r, REAL & g, REAL & b );

//! The glowing grid that sits on the neon floor.
//!
//! Lives here rather than in the display code because the camera preview draws
//! the same floor: two copies of this would be two floors that slowly stopped
//! matching, and a preview that does not match is worse than none.
void rc_PaintNeonGrid();
bool rc_NeonWalls();

//! Changes whenever anything about the arena walls' paint changes.
//!
//! Those walls are recorded once into a list and replayed from it every frame
//! after, which is why they are cheap and also why a new colour did nothing at
//! all: the recording still had the old one in it. Whoever draws them compares
//! this against what it last recorded and throws the recording away when they
//! differ.
unsigned rc_RimPaintGeneration();
