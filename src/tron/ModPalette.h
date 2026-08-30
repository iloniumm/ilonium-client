// Somewhere to type what you want instead of hunting for where it lives.
//
// The client had grown a good deal of ground: ten places down the left, nine
// sections inside the settings, and the only way to any of them was to know
// beforehand which one it was under. That is fine for whoever built it and
// slow for everybody else.
//
// This is the other way round. Press the key, type two or three letters of
// what you are after, press enter. Nothing has to be remembered about where
// anything is filed, which also means sections can be reorganised later
// without anyone having to relearn the place.

#ifndef RC_MODPALETTE_H
#define RC_MODPALETTE_H

//! Called when something is chosen, with the id it was registered under.
typedef void ( * rcPaletteGo )( int what );

//! Adds somewhere to go. The category is what shows on the right of the row
//! and is also searched, so typing "settings" finds every section of them.
void rc_PaletteAdd( int what, char const * label, char const * category );

//! Who to tell when a choice is made.
void rc_PaletteHandler( rcPaletteGo go );

//! Shows or hides it. Opening always starts from an empty query: the fastest
//! path to a place should not depend on what was typed last time.
void rc_PaletteToggle();

void rc_PaletteClose();
bool rc_PaletteVisible();

//! Draws it and handles the keys. Call once a frame while the menu is up, and
//! before anything that also wants the keyboard.
void rc_PaletteDraw();

#endif
