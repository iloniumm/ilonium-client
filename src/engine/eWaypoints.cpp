#include "eWaypoints.h"
#include "eCoord.h"
#include "tDirectories.h"
#include "tConsole.h"
#include "rConsole.h"

#include <stdio.h>
#include <string.h>

namespace
{

std::vector< rcWaypoint > kept;
int at = -1;
bool loaded = false;

std::string Where()
{
    return std::string( (char const *)rc_InternalPath( "noclip_waypoints.txt" ) );
}

}

std::vector< rcWaypoint > const & rc_Waypoints()
{
    if ( !loaded )
        rc_WaypointsLoad();

    return kept;
}

int rc_WaypointCurrent()
{
    return at;
}

void rc_WaypointAdd()
{
    if ( !loaded )
        rc_WaypointsLoad();

    rcWaypoint one;
    rc_NoclipPose( one.x, one.y, one.z, one.yaw, one.pitch, one.fov );

    kept.push_back( one );
    at = (int)kept.size() - 1;

    rc_WaypointsSave();

    tString said;
    said << "Waypoint " << (int)kept.size() << " saved";
    sr_con.DoCenterDisplay( said, 2.0, 0.3, 1.0, 0.8 );
}

void rc_WaypointDelete( int which )
{
    if ( which < 0 || which >= (int)kept.size() )
        return;

    kept.erase( kept.begin() + which );

    // Whatever was being used keeps being used where that still means
    // something, rather than the selection jumping somewhere else.
    if ( at == which )
        at = -1;
    else if ( at > which )
        --at;

    rc_WaypointsSave();
}

void rc_WaypointGoTo( int which )
{
    if ( which < 0 || which >= (int)kept.size() )
        return;

    rcWaypoint const & one = kept[ which ];

    rc_NoclipSetPose( one.x, one.y, one.z, one.yaw, one.pitch, one.fov );

    at = which;
}

void rc_WaypointStep( int by )
{
    if ( kept.empty() )
        return;

    int next = at + by;

    // Round the ends rather than stopping at them: stepping through a handful
    // of angles is the whole use of this, and a wall at each end turns it into
    // two keys instead of one.
    int many = (int)kept.size();
    while ( next < 0 )
        next += many;
    next %= many;

    rc_WaypointGoTo( next );
}

void rc_WaypointName( int which, char const * name )
{
    if ( which < 0 || which >= (int)kept.size() )
        return;

    kept[ which ].name = name ? name : "";
    rc_WaypointsSave();
}

void rc_WaypointsLoad()
{
    if ( loaded )
        return;
    loaded = true;

    FILE * f = fopen( Where().c_str(), "r" );
    if ( !f )
        return;

    char line[ 512 ];
    while ( fgets( line, sizeof( line ), f ) )
    {
        if ( line[0] == '#' )
            continue;

        rcWaypoint one;
        char name[ 200 ] = { 0 };

        int got = sscanf( line, "%f %f %f %f %f %f %199[^\n]",
                          &one.x, &one.y, &one.z, &one.yaw, &one.pitch, &one.fov, name );

        if ( got >= 6 )
        {
            one.name = name;
            kept.push_back( one );
        }
    }

    fclose( f );
}

void rc_WaypointsSave()
{
    std::string path = Where();
    std::string temp = path + ".new";

    FILE * f = fopen( temp.c_str(), "w" );
    if ( !f )
        return;

    fprintf( f, "# retrocycles free camera waypoints: x y z yaw pitch fov name\n" );

    for ( size_t i = 0; i < kept.size(); ++i )
        fprintf( f, "%.3f %.3f %.3f %.4f %.4f %.2f %s\n",
                 kept[i].x, kept[i].y, kept[i].z,
                 kept[i].yaw, kept[i].pitch, kept[i].fov,
                 kept[i].name.c_str() );

    fclose( f );

    remove( path.c_str() );
    rename( temp.c_str(), path.c_str() );
}
