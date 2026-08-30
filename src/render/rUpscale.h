// Making the game's textures bigger than they were drawn.
//
// The art in this game was cut for screens a quarter the size of the ones it
// runs on now. A 128 pixel wall texture stretched across a modern display is
// eight screen pixels per texel, and the card's own filtering has two choices
// about that, both bad: leave it blocky, or blur it into mush. Neither is what
// the artist drew.
//
// So the enlarging happens here, in software, once, on the way to the card -
// where there is time to look at the picture and decide how it wants to grow.
// Two families do the work:
//
//   - Flat art with hard edges - most of the walls, the cockpit, the fonts -
//     grows by an edge-directed rule that keeps a diagonal a diagonal instead
//     of turning it into a staircase or a smudge.
//   - Continuous art - the floors, the skies, anything photographed or painted
//     with gradients - grows by a windowed sinc, which is what you would use
//     on a photograph and is wrong for the first group.
//
// Which family a texture belongs to is not guessed from its name. It is
// measured: pixel art has long runs of exactly equal neighbours and few
// distinct colours, and continuous art has almost none of either.

#ifndef RC_RUPSCALE_H
#define RC_RUPSCALE_H

#include "defs.h"

struct SDL_Surface;

//! How the enlarging should be chosen.
enum rcUpscaleMode
{
    rcUpscale_Auto = 0,   //!< measure each texture and pick for it
    rcUpscale_Sharp,      //!< edge directed, always
    rcUpscale_Smooth      //!< windowed sinc, always
};

//! An enlarged copy of a surface, or nothing when it should be left alone.
//!
//! The caller owns what comes back and frees it with SDL_DestroySurface. A
//! null return is the normal answer for a texture that is already big enough,
//! for a factor of one, and for anything the card could not hold afterwards -
//! it means "upload what you already have".
//!
//! @param from    the surface as loaded, in whatever depth it happens to be
//! @param ceiling the largest edge the card will accept, zero for no limit
SDL_Surface * rc_UpscaleSurface( SDL_Surface * from, int ceiling );

//! The factor textures are being grown by. One means the feature is off.
int rc_UpscaleFactor();

//! The stored settings, for whatever wants to put them on screen.
int  rc_UpscaleFactorSetting();
void rc_UpscaleFactorSet( int factor );
int  rc_UpscaleModeSetting();
void rc_UpscaleModeSet( int mode );

//! Whether the settings have moved since the textures on the card were built.
//!
//! Enlarging happens on the way to the card, so a texture already there keeps
//! whatever size it was given. Somebody has to notice and reload them, and it
//! should not happen on every frame of a dragged slider.
bool rc_UpscaleDirty();
void rc_UpscaleClean();

#endif
