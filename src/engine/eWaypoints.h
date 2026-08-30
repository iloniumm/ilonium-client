// Places worth coming back to, while flying the free camera.
//
// The camera can go anywhere, which is most of the point of it and also its
// problem: finding the one angle that showed a round properly means flying
// there again from memory every time. A waypoint is that angle written down -
// where the camera stood, where it was looking and how wide - and switching
// between them is instant.
//
// They outlive the session. An angle somebody spent five minutes finding and
// loses on the way out is worse than not having saved it at all, because they
// will have expected it to be there.

#ifndef RC_EWAYPOINTS_H
#define RC_EWAYPOINTS_H

#include "defs.h"

#include <string>
#include <vector>

//! Where the free camera is standing, and putting it somewhere else.
//!
//! The camera keeps its own state to itself, which is right - this is the one
//! seam it needs to open, rather than five variables anybody could reach into.
void rc_NoclipPose( REAL & x, REAL & y, REAL & z, REAL & yaw, REAL & pitch, REAL & fov );
void rc_NoclipSetPose( REAL x, REAL y, REAL z, REAL yaw, REAL pitch, REAL fov );

struct rcWaypoint
{
    REAL x, y, z;
    REAL yaw, pitch, fov;
    std::string name;

    rcWaypoint() : x( 0 ), y( 0 ), z( 0 ), yaw( 0 ), pitch( 0 ), fov( 90 ) {}
};

//! All of them, oldest first.
std::vector< rcWaypoint > const & rc_Waypoints();

//! Which one the camera was last sent to, or -1 when it has wandered off on
//! its own since.
int rc_WaypointCurrent();

//! Writes down where the camera is standing now.
void rc_WaypointAdd();

//! Forgets one, by position in the list. Anything else keeps its own place -
//! deleting the third should not renumber the fourth out from under somebody
//! who is halfway through using it.
void rc_WaypointDelete( int which );

//! Sends the camera to one.
void rc_WaypointGoTo( int which );

//! The next or previous one along, wrapping round the ends.
void rc_WaypointStep( int by );

//! Renames one. Empty means it goes back to being numbered.
void rc_WaypointName( int which, char const * name );

void rc_WaypointsLoad();
void rc_WaypointsSave();

#endif
