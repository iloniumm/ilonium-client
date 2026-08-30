#include "eFloodlights.h"

#include "rGL.h"
#include "rSDL.h"
#include "rRender.h"
#include "rScreen.h"
#include "eAdvWall.h"
#include "eCoord.h"
#include "eTimer.h"
#include "tConfiguration.h"

#include <math.h>
#include <vector>

bool sg_floodlights = false;
static tConfItem< bool > c_on( "MOD_FLOODLIGHTS", sg_floodlights );

// Eight reads as a ring without crowding a small arena. Below four they look
// like an accident; above sixteen the pools run into one another and the floor
// is simply brighter, which is not the same thing.
int sg_floodlightCount = 8;
static tConfItem< int > c_count( "MOD_FLOODLIGHT_COUNT", sg_floodlightCount );

// 0 lights the whole floor from the corners, 1 aims everything the game
// nominated - the sumo zone - and 2 puts one mast behind each base and points
// it at the zone between them.
int sg_floodlightMode = 0;
static tConfItem< int > c_mode( "MOD_FLOODLIGHT_MODE", sg_floodlightMode );

// Stadium masts, not lamp posts. Tall enough that the head clears the rim by a
// long way and the beam comes down at an angle you can see across the arena.
REAL sg_floodlightHeight = 78;
static tConfItem< REAL > c_height( "MOD_FLOODLIGHT_HEIGHT", sg_floodlightHeight );

// How far into the arena the pool reaches. The lamps lean inwards, so this is
// also roughly how far from the wall the bright part lands.
// How wide the pool is where it lands. The throw itself is however far the
// target happens to be - a corner mast lighting the middle of the arena is
// throwing a hundred and twenty units, and that is the point of it.
REAL sg_floodlightReach = 70;
static tConfItem< REAL > c_reach( "MOD_FLOODLIGHT_REACH", sg_floodlightReach );

REAL sg_floodlightBright = 1.0;
static tConfItem< REAL > c_bright( "MOD_FLOODLIGHT_BRIGHT", sg_floodlightBright );

bool sg_floodlightShafts = true;
static tConfItem< bool > c_shafts( "MOD_FLOODLIGHT_SHAFTS", sg_floodlightShafts );

// Warm white by default: the arena's own light is cold, and a lamp that is the
// same colour as everything else does not read as a lamp.
REAL sg_floodR = 1.00, sg_floodG = 0.93, sg_floodB = 0.78;
static tConfItem< REAL > c_r( "MOD_FLOODLIGHT_R", sg_floodR );
static tConfItem< REAL > c_g( "MOD_FLOODLIGHT_G", sg_floodG );
static tConfItem< REAL > c_b( "MOD_FLOODLIGHT_B", sg_floodB );

bool rc_Floodlights()
{
    return sg_floodlights;
}

#ifndef DEDICATED

namespace
{

struct Lamp
{
    eCoord foot;      //!< where the mast meets the ground
    eCoord look;      //!< the point on the floor it is pointed at
    eCoord aim;       //!< unit vector from foot towards look
    eCoord across;    //!< unit vector across the aim
    REAL   spread;    //!< half width of the pool where it lands
};

std::vector< eCoord > s_targets;
std::vector< REAL >   s_targetSize;

std::vector< Lamp > s_lamps;
int  s_placedFor = -1;
int  s_placedCount = 0;

//! The arena's corners, taken from where its rim segments meet.
//!
//! The rim arrives as an unordered pile of segments. Their endpoints are the
//! polygon's vertices, each shared by two segments, so collecting the
//! endpoints and merging the duplicates gives the corners of a square, an
//! octagon or anything somebody drew by hand - without assuming any of them.
void Corners( std::vector< eCoord > & out, eCoord & middle )
{
    out.clear();
    middle = eCoord( 0, 0 );

    int segments = se_rimWalls.Len();
    int counted = 0;

    for ( int i = 0; i < segments; ++i )
    {
        eWallRim * w = se_rimWalls( i );
        if ( !w ) continue;

        for ( int e = 0; e < 2; ++e )
        {
            eCoord p = w->EndPoint( e );
            middle = middle + p;
            ++counted;

            bool seen = false;
            for ( size_t k = 0; k < out.size(); ++k )
                if ( ( out[k] - p ).NormSquared() < 4.0f )
                {
                    seen = true;
                    break;
                }

            if ( !seen )
                out.push_back( p );
        }
    }

    if ( counted )
        middle = middle * ( 1.0f / counted );
}

//! The rim point nearest a place - where a mast belongs if it is to stand
//! behind something and shine at it.
bool NearestRim( eCoord const & to, eCoord & found )
{
    REAL best = 1e30f;
    bool any = false;

    for ( int i = se_rimWalls.Len() - 1; i >= 0; --i )
    {
        eWallRim * w = se_rimWalls( i );
        if ( !w ) continue;

        eCoord a = w->EndPoint(0), b = w->EndPoint(1);
        eCoord along = b - a;
        REAL len2 = along.NormSquared();
        if ( len2 < 1e-4f ) continue;

        REAL t = ( ( to - a ) * along ) / len2;
        if ( t < 0 ) t = 0;
        if ( t > 1 ) t = 1;

        eCoord on = a + along * t;
        REAL d = ( on - to ).NormSquared();

        if ( d < best )
        {
            best = d;
            found = on;
            any = true;
        }
    }

    return any;
}

void Stand( eCoord const & foot, eCoord const & look, REAL spread )
{
    eCoord aim = look - foot;
    REAL len = aim.Norm();
    if ( len < 1.0f )
        return;

    Lamp lamp;
    lamp.foot = foot;
    lamp.look = look;
    lamp.aim = aim * ( 1.0f / len );
    lamp.across = eCoord( -lamp.aim.y, lamp.aim.x );
    lamp.spread = spread;

    s_lamps.push_back( lamp );
}

void Place()
{
    s_lamps.clear();

    std::vector< eCoord > corners;
    eCoord middle;
    Corners( corners, middle );

    if ( corners.empty() )
        return;

    REAL spread = sg_floodlightReach;
    if ( spread < 8 ) spread = 8;

    // ---- two masts, one behind each base, both pointed at what lies between
    if ( sg_floodlightMode == 2 && s_targets.size() >= 2 )
    {
        eCoord between( 0, 0 );
        for ( size_t i = 0; i < s_targets.size(); ++i )
            between = between + s_targets[i];
        between = between * ( 1.0f / s_targets.size() );

        for ( size_t i = 0; i < s_targets.size() && i < 4; ++i )
        {
            // behind the base, on the wall nearest it, looking across
            eCoord back = s_targets[i] + ( s_targets[i] - between );
            eCoord foot;
            if ( NearestRim( back, foot ) )
                Stand( foot, between, spread );
        }

        if ( !s_lamps.empty() )
            return;
    }

    // ---- everything the game nominated, lit from the corners
    if ( sg_floodlightMode == 1 && !s_targets.empty() )
    {
        for ( size_t c = 0; c < corners.size(); ++c )
        {
            eCoord const & at = s_targets[ c % s_targets.size() ];
            REAL want = s_targetSize.empty() ? spread : s_targetSize[ c % s_targetSize.size() ] * 1.6f;
            if ( want < spread * 0.5f ) want = spread * 0.5f;
            Stand( corners[c], at, want );
        }

        if ( !s_lamps.empty() )
            return;
    }

    // ---- nothing to aim at: every corner throws across the arena, each one
    // landing short of the far side so the floor is covered rather than all
    // four piling into the same spot
    for ( size_t c = 0; c < corners.size(); ++c )
    {
        eCoord away = middle - corners[c];
        REAL len = away.Norm();
        if ( len < 1 ) continue;
        away = away * ( 1.0f / len );

        eCoord side( -away.y, away.x );
        REAL swing = ( c % 2 ) ? 0.22f : -0.22f;

        eCoord look = corners[c] + away * ( len * 1.15f ) + side * ( len * swing );
        Stand( corners[c], look, spread );
    }
}

void Ensure()
{
    int now = se_rimWalls.Len();
    int shape = now * 37 + sg_floodlightMode * 7 + (int)s_targets.size();

    if ( shape == s_placedFor && !s_lamps.empty() )
        return;

    s_placedFor = shape;
    Place();
}

//! Additive, no depth writing: light adds to what is already there and never
//! hides anything behind it.
//! What one lamp is allowed to put on the floor.
//!
//! The pools are added together, and that was harmless while the frame stopped
//! at white: six lamps overlapping simply reached it and stayed. Against a
//! float buffer the sum keeps climbing to several times white, and the tonemap
//! hands the whole arena back as one flat sheet of colour. So each pool now
//! contributes about a third of what it used to, which leaves the sum of all
//! of them under one where they overlap.
static REAL const kPoolShare = 0.32f;

void BeginLight()
{
    glPushAttrib( GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
    glDisable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    glDepthMask( GL_FALSE );
}

void EndLight()
{
    RenderEnd();
    glDepthMask( GL_TRUE );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glPopAttrib();
}

}

void rc_FloodlightTargets( eCoord const * points, REAL const * radii, int count )
{
    s_targets.clear();
    s_targetSize.clear();

    for ( int i = 0; i < count; ++i )
    {
        s_targets.push_back( points[i] );
        s_targetSize.push_back( radii ? radii[i] : 0 );
    }

    s_placedFor = -1;
}

void rc_FloodlightsForget()
{
    s_lamps.clear();
    s_placedFor = -1;
}

void rc_FloodlightPools()
{
    if ( !sg_floodlights )
        return;

    Ensure();
    if ( s_lamps.empty() )
        return;

    REAL bright = sg_floodlightBright;
    if ( bright < 0 ) bright = 0;
    if ( bright > 3 ) bright = 3;

    REAL reach = sg_floodlightReach;
    if ( reach < 4 ) reach = 4;

    BeginLight();

    for ( size_t i = 0; i < s_lamps.size(); ++i )
    {
        Lamp const & lamp = s_lamps[i];

        // The pool lands where the lamp is pointed, not at its feet, and it is
        // drawn out along the throw because a beam arriving at a shallow angle
        // makes an ellipse, not a circle.
        REAL along = lamp.spread * 1.55f;
        REAL wide = lamp.spread;

        for ( int pass = 0; pass < 3; ++pass )
        {
            REAL k = 1.0f - 0.32f * pass;
            REAL alpha = ( 0.07f + 0.035f * pass ) * bright * kPoolShare;
            REAL lift = 0.06f + 0.01f * pass;
            int steps = 34;

            BeginTriangleFan();
            glColor4f( sg_floodR, sg_floodG, sg_floodB, alpha );
            glVertex3f( lamp.look.x, lamp.look.y, lift );

            glColor4f( sg_floodR, sg_floodG, sg_floodB, 0.0f );
            for ( int st = 0; st <= steps; ++st )
            {
                REAL t = (REAL)st / steps * 6.2831853f;
                eCoord edge = lamp.look + lamp.aim * ( cosf( t ) * along * k )
                                        + lamp.across * ( sinf( t ) * wide * k );
                glVertex3f( edge.x, edge.y, lift );
            }
            RenderEnd();
        }
    }

    EndLight();
}

void rc_FloodlightFixtures()
{
    if ( !sg_floodlights )
        return;

    Ensure();
    if ( s_lamps.empty() )
        return;

    REAL bright = sg_floodlightBright;
    if ( bright < 0 ) bright = 0;
    if ( bright > 3 ) bright = 3;

    REAL h = sg_floodlightHeight;
    if ( h < 6 ) h = 6;

    // Which way the screen's right and up point, read off the matrix the
    // camera already set. A halo has to face the viewer, and this is cheaper
    // and less brittle than asking the camera where it is.
    GLfloat m[16];
    glGetFloatv( GL_MODELVIEW_MATRIX, m );
    eCoord right( m[0], m[4] );
    REAL rightZ = m[8];
    eCoord up( m[1], m[5] );
    REAL upZ = m[9];

    REAL const post = h * 0.030f;      // the mast thickens with its height
    REAL const rigW = h * 0.115f;      // the head is a rig, not a lantern
    REAL const rigH = h * 0.075f;

    // ---- masts and rigs, dark and solid: lit by nothing, read as
    // silhouettes, which is what a floodlight is from underneath
    glPushAttrib( GL_ENABLE_BIT );
    glDisable( GL_TEXTURE_2D );

    for ( size_t i = 0; i < s_lamps.size(); ++i )
    {
        Lamp const & lamp = s_lamps[i];
        eCoord back = lamp.aim * -1;
        eCoord side = lamp.across;
        eCoord base = lamp.foot + back * ( post * 2.0f );

        BeginQuads();
        glColor4f( 0.10f, 0.11f, 0.14f, 1.0f );
        eCoord c1 = base - side * post, c2 = base + side * post;
        glVertex3f( c1.x, c1.y, 0 );
        glVertex3f( c2.x, c2.y, 0 );
        glVertex3f( c2.x, c2.y, h );
        glVertex3f( c1.x, c1.y, h );

        glColor4f( 0.07f, 0.08f, 0.10f, 1.0f );
        eCoord d1 = base - lamp.aim * post, d2 = base + lamp.aim * post;
        glVertex3f( d1.x, d1.y, 0 );
        glVertex3f( d2.x, d2.y, 0 );
        glVertex3f( d2.x, d2.y, h );
        glVertex3f( d1.x, d1.y, h );
        RenderEnd();

        eCoord rig = base + lamp.aim * ( rigW * 0.35f );
        REAL top = h + rigH, bot = h - rigH;

        BeginQuads();
        glColor4f( 0.13f, 0.14f, 0.17f, 1.0f );
        eCoord e1 = rig - side * rigW, e2 = rig + side * rigW;
        glVertex3f( e1.x, e1.y, bot );
        glVertex3f( e2.x, e2.y, bot );
        glVertex3f( e2.x, e2.y, top );
        glVertex3f( e1.x, e1.y, top );
        RenderEnd();

        // the bank of lamps in the rig, the one part that is actually bright
        eCoord face = rig + lamp.aim * ( rigW * 0.10f );
        int const cells = 4;
        BeginQuads();
        glColor4f( sg_floodR, sg_floodG, sg_floodB, 1.0f );
        for ( int c = 0; c < cells; ++c )
        {
            REAL u0 = -rigW * 0.86f + ( 2 * rigW * 0.86f ) * c / cells;
            REAL u1 = u0 + ( 2 * rigW * 0.86f ) / cells * 0.72f;
            eCoord f1 = face + side * u0, f2 = face + side * u1;
            glVertex3f( f1.x, f1.y, bot + rigH * 0.30f );
            glVertex3f( f2.x, f2.y, bot + rigH * 0.30f );
            glVertex3f( f2.x, f2.y, top - rigH * 0.30f );
            glVertex3f( f1.x, f1.y, top - rigH * 0.30f );
        }
        RenderEnd();
    }

    glPopAttrib();

    // ---- the halo, and the beam it throws down the arena
    BeginLight();

    for ( size_t i = 0; i < s_lamps.size(); ++i )
    {
        Lamp const & lamp = s_lamps[i];
        eCoord base = lamp.foot + lamp.aim * ( -post * 2.0f );
        eCoord head = base + lamp.aim * ( rigW * 0.45f );
        REAL headZ = h;

        for ( int pass = 0; pass < 2; ++pass )
        {
            // A halo wide enough to wash the sky stops reading as a lamp and
            // starts reading as fog over the whole arena.
            REAL radius = pass ? rigW * 1.7f : rigW * 0.85f;
            REAL alpha = ( pass ? 0.10f : 0.55f ) * bright;
            int steps = pass ? 22 : 14;

            BeginTriangleFan();
            glColor4f( sg_floodR, sg_floodG, sg_floodB, alpha );
            glVertex3f( head.x, head.y, headZ );

            glColor4f( sg_floodR, sg_floodG, sg_floodB, 0.0f );
            for ( int st = 0; st <= steps; ++st )
            {
                REAL t = (REAL)st / steps * 6.2831853f;
                REAL cx = cosf( t ) * radius, cy = sinf( t ) * radius;
                eCoord p = head + right * cx + up * cy;
                glVertex3f( p.x, p.y, headZ + rightZ * cx + upZ * cy );
            }
            RenderEnd();
        }

        if ( sg_floodlightShafts )
        {
            // The beam as a sheet turned to face the camera: a cone seen edge
            // on disappears and this never does. It fades out along its length
            // so the air it crosses looks like air rather than like a solid.
            eCoord n1 = head - lamp.across * ( rigW * 0.7f );
            eCoord n2 = head + lamp.across * ( rigW * 0.7f );
            eCoord w1 = lamp.look - lamp.across * lamp.spread;
            eCoord w2 = lamp.look + lamp.across * lamp.spread;

            int const rungs = 5;
            for ( int r = 0; r < rungs; ++r )
            {
                REAL t0 = (REAL)r / rungs, t1 = (REAL)( r + 1 ) / rungs;
                REAL a0 = ( 1.0f - t0 ) * 0.075f * bright;
                REAL a1 = ( 1.0f - t1 ) * 0.075f * bright;

                eCoord p0a = n1 + ( w1 - n1 ) * t0, p0b = n2 + ( w2 - n2 ) * t0;
                eCoord p1a = n1 + ( w1 - n1 ) * t1, p1b = n2 + ( w2 - n2 ) * t1;
                REAL z0 = headZ * ( 1 - t0 ) + 0.1f * t0;
                REAL z1 = headZ * ( 1 - t1 ) + 0.1f * t1;

                BeginQuads();
                glColor4f( sg_floodR, sg_floodG, sg_floodB, a0 );
                glVertex3f( p0a.x, p0a.y, z0 );
                glVertex3f( p0b.x, p0b.y, z0 );
                glColor4f( sg_floodR, sg_floodG, sg_floodB, a1 );
                glVertex3f( p1b.x, p1b.y, z1 );
                glVertex3f( p1a.x, p1a.y, z1 );
                RenderEnd();
            }
        }
    }

    EndLight();
}

#else

void rc_FloodlightPools(){}
void rc_FloodlightFixtures(){}
void rc_FloodlightsForget(){}
void rc_FloodlightTargets( eCoord const *, REAL const *, int ){}

#endif
