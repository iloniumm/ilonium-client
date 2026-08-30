// The extras: a place to look things up and a place to make things.
//
// These are not settings and they are not gameplay - they are the small tools
// people currently keep in a browser tab beside the game. Having them inside
// it means they work offline, they match this build rather than a page written
// against another one, and they can read what the client already knows.

#ifndef RC_MODTOOLS_H
#define RC_MODTOOLS_H

//! Draws the command reference. Everything in it comes from the client's own
//! registry of settings, so it is exactly what this build understands - not a
//! list copied from somewhere that will drift out of date the first time
//! anything is added.
void rc_DrawCommandFinder( float width );

//! Draws the gradient name maker.
void rc_DrawGradientMaker( float width );

//! Draws the camera tool: a view from the seat and the numbers behind it.
void rc_DrawCameraTool( float width );

#endif
