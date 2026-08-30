/*

*************************************************************************

RetroCycles

**************************************************************************

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

***************************************************************************

*/

#ifndef ArmageTron_SHELL_H
#define ArmageTron_SHELL_H

#include <string>

// Runs a command line and hands back everything it wrote to stdout.
//
// The point of this existing at all is Windows: popen() there starts a console
// host, which flashes a window on screen and takes the keyboard focus with it.
// The chat asks the server for messages every few seconds, so a game running
// full screen was being pulled out of itself over and over. This starts the
// process with no window attached instead.
//
// Returns false if the process could not be started; output is whatever it
// managed to produce either way.
bool rc_RunHidden( const std::string & command, std::string & output );

#endif
