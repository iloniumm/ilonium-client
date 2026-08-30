// The one place colour, spacing and timing are decided.
//
// What made the old screen look thrown together was not a shortage of effects.
// It was that every panel invented its own values: a border at full strength
// here, a slightly different black there, one corner rounded to eight and its
// neighbour to twenty four. Nothing agreed with anything, so nothing looked
// deliberate.
//
// So there are rules, and they are short:
//
//   - Background is a ladder. Six steps from the page to the raised surface,
//     each one barely lighter than the last. Depth comes from the ladder, not
//     from outlines.
//   - An accent appears in exactly three strengths: solid for the one thing
//     that matters on screen, a line at just over half, and a wash at a
//     seventh. Never a solid fill behind text.
//   - Borders are hairlines. A border you can name the colour of is too loud.
//   - One duration and one curve for everything that moves.
//
// Anything that needs a colour asks here. Nothing writes its own.

#ifndef RC_MODTHEME_H
#define RC_MODTHEME_H

#include "imgui.h"

//! An accent in the three strengths it is allowed to appear in.
struct rcAccent
{
    ImU32 solid;    //!< the thing itself
    ImU32 bright;   //!< lifted, for text on top of the wash
    ImU32 line;     //!< a rule or an outline that is meant to be seen
    ImU32 wash;     //!< a fill behind something, barely there
};

struct rcTheme
{
    // the ladder, darkest first
    ImU32 page;
    ImU32 sunken;
    ImU32 surface;
    ImU32 surfaceHi;
    ImU32 raised;
    ImU32 raisedHi;

    // hairlines
    ImU32 line;
    ImU32 lineStrong;

    // type
    ImU32 text;
    ImU32 textDim;
    ImU32 textMute;

    // accents
    rcAccent primary;    //!< the identity colour, used sparingly
    rcAccent second;     //!< live values, links, anything cyan wants to be
    rcAccent gold;       //!< rare things only: records, first place

    // meaning
    ImU32 good;
    ImU32 bad;
    ImU32 warn;

    // shape
    float radiusSmall;
    float radiusMedium;
    float radiusLarge;
    float radiusPill;

    // rhythm: every gap in the interface is a multiple of this
    float unit;

    // motion
    float quick;         //!< seconds, for hover and press
    float settle;        //!< seconds, for things arriving on screen
};

//! The theme. Every colour in the interface comes from here.
rcTheme const & rc_Theme();

//! Pushes the whole look onto an ImGui style: rounding, spacing, borders and
//! the colour table. Called once at start up and again when the scale changes.
void rc_ApplyStyle( ImGuiStyle & style, float scale );

//! How far along a widget is towards being hovered or held, from zero to one,
//! remembered per widget and eased. Feeding an interface animation from this
//! rather than from a plain boolean is most of what separates something that
//! feels made from something that blinks.
//! @param id     anything unique and stable for the widget
//! @param toward where it should end up, usually zero or one
//! @param speed  seconds to travel the whole way; zero takes the theme's
float rc_Ease( ImGuiID id, float toward, float speed = 0.0f );

//! Text with the letters held apart. Small capitals set this way read as a
//! label rather than as something somebody forgot to finish, and it is the one
//! typographic trick that does the most for the least.
void rc_Tracked( char const * text, float tracking, ImU32 colour );

//! The same, measured rather than drawn.
float rc_TrackedWidth( char const * text, float tracking );

//! A colour with its opacity scaled, keeping everything else.
ImU32 rc_Fade( ImU32 colour, float alpha );

//! Somewhere between two colours.
ImU32 rc_Mix( ImU32 from, ImU32 to, float t );

//! Throws a handful of hearts across the screen. There is no reason for this
//! beyond the one that matters: somebody who finds it should feel like they
//! found something.
void rc_HeartBurst();

//! Says something once, in the corner, and gets out of the way.
//!
//! There was nowhere for the client to tell somebody a small thing - that they
//! just beat their own record, that a setting took effect - without either
//! interrupting them with a dialog or writing to a console nobody reads. This
//! is that place: it arrives, it is legible for a few seconds, it leaves.
//!
//! @param sound one of the interface sounds, or nothing for silence
void rc_Toast( char const * text, char const * sound = 0 );

//! Draws whatever is queued. Called once a frame, last.
void rc_DrawToasts();

//! The pointer, drawn rather than borrowed from the system.
//!
//! The one the desktop provides cannot answer anything: it does not know a
//! button went down, it does not know what it is over, and it belongs to
//! somebody else's design. This one is a ring with a dot, it tightens when
//! pressed and leaves a short trace of where it has been.
//! @return false when it declined to draw, and the system's should be shown
//!         instead - during a stall there are no frames to move ours on, and
//!         a pointer that sticks while the hand moves is worse than a plain
//!         one that keeps up.
bool rc_DrawCursor( bool pressed );

//! Everything behind the interface: the ground the panels sit on.
//!
//! What was there was flat black with a few dots drifting over it, which is
//! not a background so much as an absence of one. This gives the screen a
//! floor and a horizon, the way the game itself has, and two slow lights that
//! never quite arrive anywhere - so the eye has something to rest on without
//! anything ever asking for attention.
void rc_DrawBackdrop( ImDrawList * dl, ImVec2 at, ImVec2 size, float alpha );

//! How far a newly shown page has settled, from zero to one.
//!
//! Content that simply swaps is the single clearest sign of something put
//! together quickly: the eye gets no hint that it moved, so every change of
//! section feels like a jump cut. Give the same change a fifth of a second of
//! arrival and the whole thing reads as deliberate.
//!
//! @param token anything that differs between pages - a tab index will do
float rc_PageEase( int token );

//! A heart, drawn rather than pasted in. There is no image behind this: the
//! outline is the curve itself, so it stays clean at any size, takes any
//! colour, and costs nothing to ship.
//! @param size  distance from the centre to the point at the bottom
//! @param lean  radians, so a drifting one can turn as it goes
void rc_DrawHeart( ImDrawList * dl, ImVec2 at, float size, float lean, ImU32 colour );

#endif
