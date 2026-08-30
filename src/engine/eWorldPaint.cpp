#include "eWorldPaint.h"
#include "eTimer.h"
#include "tConfiguration.h"
#include "rGL.h"
#include "rRender.h"
#include <math.h>

// The values the floor was drawn with before any of this was a setting, so a
// client that has never been touched looks exactly as it did.
REAL sg_floorR = 0.012f, sg_floorG = 0.012f, sg_floorB = 0.035f;
REAL sg_gridR  = 0.0f,   sg_gridG  = 0.80f,  sg_gridB  = 1.0f;
REAL sg_majorR = 0.90f,  sg_majorG = 0.10f,  sg_majorB = 0.95f;

static tConfItem<REAL> cfr( "MOD_FLOOR_R", sg_floorR );
static tConfItem<REAL> cfg( "MOD_FLOOR_G", sg_floorG );
static tConfItem<REAL> cfb( "MOD_FLOOR_B", sg_floorB );

static tConfItem<REAL> cgr( "MOD_GRID_R", sg_gridR );
static tConfItem<REAL> cgg( "MOD_GRID_G", sg_gridG );
static tConfItem<REAL> cgb( "MOD_GRID_B", sg_gridB );

static tConfItem<REAL> cmr( "MOD_GRID_MAJOR_R", sg_majorR );
static tConfItem<REAL> cmg( "MOD_GRID_MAJOR_G", sg_majorG );
static tConfItem<REAL> cmb( "MOD_GRID_MAJOR_B", sg_majorB );

rcPaint const & rc_FloorPaint()
{
    static rcPaint paint;
    paint.r = sg_floorR;
    paint.g = sg_floorG;
    paint.b = sg_floorB;
    return paint;
}

rcPaint const & rc_GridPaint()
{
    static rcPaint paint;
    paint.r = sg_gridR;
    paint.g = sg_gridG;
    paint.b = sg_gridB;
    return paint;
}

rcPaint const & rc_GridMajorPaint()
{
    static rcPaint paint;
    paint.r = sg_majorR;
    paint.g = sg_majorG;
    paint.b = sg_majorB;
    return paint;
}

// The trail. One colour is what the game has always done - the rider's own -
// and this leaves that alone unless somebody asks for otherwise. Two or three
// are blended along the wall's length, which is the part worth having: a wall
// that changes as it runs away from you says how old it is at a glance.

bool sg_trailPainted = false;
int  sg_trailColours = 2;

REAL sg_trail1R = 1.00f, sg_trail1G = 0.20f, sg_trail1B = 0.55f;
REAL sg_trail2R = 0.25f, sg_trail2G = 0.90f, sg_trail2B = 1.00f;
REAL sg_trail3R = 1.00f, sg_trail3G = 0.78f, sg_trail3B = 0.35f;

static tConfItem<bool> ctp( "MOD_TRAIL_PAINT", sg_trailPainted );
static tConfItem<int>  ctc( "MOD_TRAIL_COLOURS", sg_trailColours );

static tConfItem<REAL> ct1r( "MOD_TRAIL1_R", sg_trail1R );
static tConfItem<REAL> ct1g( "MOD_TRAIL1_G", sg_trail1G );
static tConfItem<REAL> ct1b( "MOD_TRAIL1_B", sg_trail1B );

static tConfItem<REAL> ct2r( "MOD_TRAIL2_R", sg_trail2R );
static tConfItem<REAL> ct2g( "MOD_TRAIL2_G", sg_trail2G );
static tConfItem<REAL> ct2b( "MOD_TRAIL2_B", sg_trail2B );

static tConfItem<REAL> ct3r( "MOD_TRAIL3_R", sg_trail3R );
static tConfItem<REAL> ct3g( "MOD_TRAIL3_G", sg_trail3G );
static tConfItem<REAL> ct3b( "MOD_TRAIL3_B", sg_trail3B );

bool rc_TrailPainted()
{
    return sg_trailPainted;
}

int rc_TrailColourCount()
{
    if ( sg_trailColours < 1 ) return 1;
    if ( sg_trailColours > 3 ) return 3;
    return sg_trailColours;
}

rcPaint rc_TrailPaint( REAL along )
{
    if ( along < 0 ) along = 0;
    if ( along > 1 ) along = 1;

    rcPaint one   = { sg_trail1R, sg_trail1G, sg_trail1B };
    rcPaint two   = { sg_trail2R, sg_trail2G, sg_trail2B };
    rcPaint three = { sg_trail3R, sg_trail3G, sg_trail3B };

    int count = rc_TrailColourCount();
    if ( count == 1 )
        return one;

    // With three, the middle one owns the middle of the wall and each half is
    // a blend of two, which is the only arrangement that does not leave one of
    // them as a stripe nobody asked for.
    rcPaint from = one, to = two;
    REAL t = along;

    if ( count == 3 )
    {
        if ( along < 0.5f )
        {
            from = one; to = two;
            t = along * 2.0f;
        }
        else
        {
            from = two; to = three;
            t = ( along - 0.5f ) * 2.0f;
        }
    }

    rcPaint mixed;
    mixed.r = from.r + ( to.r - from.r ) * t;
    mixed.g = from.g + ( to.g - from.g ) * t;
    mixed.b = from.b + ( to.b - from.b ) * t;
    return mixed;
}


// The arena's own walls. Same idea as the trail and the same reasoning behind
// the blend, but a different thing entirely: these belong to the map rather
// than to a player, so painting them is nobody's advantage and everybody's
// choice.

bool sg_rimPainted = false;
int  sg_rimColours = 2;

REAL sg_rim1R = 0.10f, sg_rim1G = 0.35f, sg_rim1B = 0.85f;
REAL sg_rim2R = 0.70f, sg_rim2G = 0.15f, sg_rim2B = 0.90f;
REAL sg_rim3R = 0.10f, sg_rim3G = 0.85f, sg_rim3B = 0.80f;

static tConfItem<bool> crp( "MOD_RIM_PAINT", sg_rimPainted );
static tConfItem<int>  crc( "MOD_RIM_COLOURS", sg_rimColours );

static tConfItem<REAL> cr1r( "MOD_RIM1_R", sg_rim1R );
static tConfItem<REAL> cr1g( "MOD_RIM1_G", sg_rim1G );
static tConfItem<REAL> cr1b( "MOD_RIM1_B", sg_rim1B );

static tConfItem<REAL> cr2r( "MOD_RIM2_R", sg_rim2R );
static tConfItem<REAL> cr2g( "MOD_RIM2_G", sg_rim2G );
static tConfItem<REAL> cr2b( "MOD_RIM2_B", sg_rim2B );

static tConfItem<REAL> cr3r( "MOD_RIM3_R", sg_rim3R );
static tConfItem<REAL> cr3g( "MOD_RIM3_G", sg_rim3G );
static tConfItem<REAL> cr3b( "MOD_RIM3_B", sg_rim3B );

bool rc_RimPainted()
{
    return sg_rimPainted;
}

rcPaint rc_RimPaint( REAL along )
{
    if ( along < 0 ) along = 0;
    if ( along > 1 ) along = 1;

    rcPaint one   = { sg_rim1R, sg_rim1G, sg_rim1B };
    rcPaint two   = { sg_rim2R, sg_rim2G, sg_rim2B };
    rcPaint three = { sg_rim3R, sg_rim3G, sg_rim3B };

    int count = sg_rimColours;
    if ( count < 1 ) count = 1;
    if ( count > 3 ) count = 3;

    if ( count == 1 )
        return one;

    rcPaint from = one, to = two;
    REAL t = along;

    if ( count == 3 )
    {
        if ( along < 0.5f )
        {
            from = one; to = two;
            t = along * 2.0f;
        }
        else
        {
            from = two; to = three;
            t = ( along - 0.5f ) * 2.0f;
        }
    }

    rcPaint mixed;
    mixed.r = from.r + ( to.r - from.r ) * t;
    mixed.g = from.g + ( to.g - from.g ) * t;
    mixed.b = from.b + ( to.b - from.b ) * t;
    return mixed;
}

unsigned rc_RimPaintGeneration()
{
    // Enough to notice a change, and it does not have to be more than that:
    // the cost of being wrong is one extra recording of a few dozen walls.
    unsigned mark = sg_rimPainted ? 1u : 0u;
    mark = mark * 31u + (unsigned)( sg_rimColours );

    REAL const parts[ 9 ] = { sg_rim1R, sg_rim1G, sg_rim1B,
                              sg_rim2R, sg_rim2G, sg_rim2B,
                              sg_rim3R, sg_rim3G, sg_rim3B };

    for ( int i = 0; i < 9; ++i )
        mark = mark * 131u + (unsigned)( parts[i] * 255.0f );

    return mark;
}


// Asked for on their own, or as part of the master switch that has always
// turned them both on along with everything else.

bool sg_neonFloor = false;
bool sg_neonWalls = false;

static tConfItem<bool> cnf( "MOD_NEON_FLOOR", sg_neonFloor );
static tConfItem<bool> cnw( "MOD_NEON_WALLS", sg_neonWalls );

bool rc_NeonFloor()
{
    extern bool g_EnhancedGraphicsMode;
    return sg_neonFloor || g_EnhancedGraphicsMode;
}

// The floor had a texture made for it and no way of ever showing it: the plain
// look was shipped with the floor switched off and its texture group disabled,
// so the ground came up bare whatever was in the pack. This is the switch for
// it. The neon look is left alone - it has its own ground and its own grid,
// and putting panels under those is not what it is for.
bool sg_floorTexture = true;
static tConfItem<bool> cft( "MOD_FLOOR_TEXTURE", sg_floorTexture );

REAL sg_floorTextureLight = 0.15f;
static tConfItem<REAL> cftl( "MOD_FLOOR_TEXTURE_LIGHT", sg_floorTextureLight );

// How many grid squares one tile of the floor covers. One, the same as the
// floor has always tiled at - the panels are the change here, not the size of
// them.
REAL sg_floorTextureScale = 1.0f;
static tConfItem<REAL> cfts( "MOD_FLOOR_TEXTURE_SCALE", sg_floorTextureScale );

bool rc_FloorTexture()
{
    return sg_floorTexture;
}

REAL rc_FloorTextureScale()
{
    return sg_floorTextureScale > 0.05f ? sg_floorTextureScale : 0.05f;
}

REAL rc_FloorTextureLight()
{
    return sg_floorTextureLight > 0 ? sg_floorTextureLight : 0;
}

//! The colour the floor texture is drawn in.
//!
//! Multiplying it by the floor colour is what the flat ground is for, and that
//! colour is nearly black by design - it would leave the panels invisible. So
//! the colour is used for its hue only, and the brightness comes from the
//! setting.
void rc_FloorTextureColour( REAL & r, REAL & g, REAL & b )
{
    rcPaint const & paint = rc_FloorPaint();

    REAL top = paint.r;
    if ( paint.g > top ) top = paint.g;
    if ( paint.b > top ) top = paint.b;

    REAL const lit = rc_FloorTextureLight();

    if ( top < 0.0001f )
    {
        r = g = b = lit;
        return;
    }

    r = ( 0.55f + 0.45f * paint.r / top ) * lit;
    g = ( 0.55f + 0.45f * paint.g / top ) * lit;
    b = ( 0.55f + 0.45f * paint.b / top ) * lit;
}

void rc_PaintNeonGrid()
{
#ifndef DEDICATED
    // 2. Procedural Glowing Neon Grid Lines (additive blend)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive glowing grid lines
    glLineWidth(1.5f);

    REAL extent = 1200.0f; // Wide coverage grid
    REAL spacing = 15.0f;  // Grid square size

    float pulse = (float)(sin(se_GameTime() * 3.5) * 0.15 + 0.50);

    // Primary Cyan Grid
    {
        // The pulse rides on top of the chosen colour rather than
        // replacing part of it, so a grid set to red still breathes.
        rcPaint const & gridPaint = rc_GridPaint();
        REAL lift = 0.80f + pulse * 0.35f;
        glColor4f(gridPaint.r * lift, gridPaint.g * lift, gridPaint.b * lift,
          0.40f + pulse * 0.15f);
    }
    BeginLines();
    for (REAL x = -extent; x <= extent; x += spacing) {
        glVertex3f(x, -extent, 0.05f);
        glVertex3f(x,  extent, 0.05f);
    }
    for (REAL y = -extent; y <= extent; y += spacing) {
        glVertex3f(-extent, y, 0.05f);
        glVertex3f( extent, y, 0.05f);
    }
    RenderEnd();

    // Major Magenta Accent Lines (every 5th line)
    {
        rcPaint const & majorPaint = rc_GridMajorPaint();
        REAL lift = 0.85f + pulse * 0.15f;
        glColor4f(majorPaint.r * lift, majorPaint.g * lift, majorPaint.b * lift,
          0.55f + pulse * 0.20f);
    }
    glLineWidth(2.2f);
    BeginLines();
    for (REAL x = -extent; x <= extent; x += spacing * 5.0f) {
        glVertex3f(x, -extent, 0.08f);
        glVertex3f(x,  extent, 0.08f);
    }
    for (REAL y = -extent; y <= extent; y += spacing * 5.0f) {
        glVertex3f(-extent, y, 0.08f);
        glVertex3f( extent, y, 0.08f);
    }
    RenderEnd();

    // 3. Glowing Neon Grid Intersection Nodes (Cyan/Magenta Diamond Crosses)
    glLineWidth(1.8f);
    {
        // The crosses belong to the grid they sit on, so they take its
        // colour rather than one of their own.
        rcPaint const & nodePaint = rc_GridPaint();
        REAL lift = 0.95f;
        glColor4f(nodePaint.r * lift, nodePaint.g * lift, nodePaint.b * lift,
          0.65f + pulse * 0.25f);
    }
    BeginLines();
    REAL stepNode = spacing * 2.0f;
    REAL hs = 0.8f; // Half size of node cross
    for (REAL x = -600.0f; x <= 600.0f; x += stepNode) {
        for (REAL y = -600.0f; y <= 600.0f; y += stepNode) {
            glVertex3f(x - hs, y, 0.10f);
            glVertex3f(x + hs, y, 0.10f);
            glVertex3f(x, y - hs, 0.10f);
            glVertex3f(x, y + hs, 0.10f);
        }
    }
    RenderEnd();
#endif
}

bool rc_NeonWalls()
{
    extern bool g_EnhancedGraphicsMode;
    return sg_neonWalls || g_EnhancedGraphicsMode;
}


// The shape itself. Fourteen points is where the outline stops looking like a
// polygon at the sizes anything here draws it, and every point past that is
// spent on nothing anybody can see.

static REAL const sg_heartX[ 14 ] = {
     0.000f,  0.196f,  0.363f,  0.462f,  0.462f,  0.363f,  0.196f,
     0.000f, -0.196f, -0.363f, -0.462f, -0.462f, -0.363f, -0.196f };

static REAL const sg_heartY[ 14 ] = {
    -0.500f, -0.400f, -0.100f,  0.180f,  0.360f,  0.440f,  0.380f,
     0.230f,  0.380f,  0.440f,  0.360f,  0.180f, -0.100f, -0.400f };

int rc_HeartShape( REAL const * & xs, REAL const * & ys )
{
    xs = sg_heartX;
    ys = sg_heartY;
    return 14;
}

bool sg_heartDeaths = false;
static tConfItem<bool> chd( "MOD_HEART_DEATHS", sg_heartDeaths );

bool rc_HeartDeaths()
{
    return sg_heartDeaths;
}

bool sg_heartSparks = false;
static tConfItem<bool> chs( "MOD_HEART_SPARKS", sg_heartSparks );

int sg_heartDeathCount = 22;
int sg_heartSparkCount = 10;
static tConfItem<int> chdc( "MOD_HEART_DEATH_COUNT", sg_heartDeathCount );
static tConfItem<int> chsc( "MOD_HEART_SPARK_COUNT", sg_heartSparkCount );

int rc_HeartDeathCount()
{
    if ( sg_heartDeathCount < 1 )  return 1;
    if ( sg_heartDeathCount > 60 ) return 60;
    return sg_heartDeathCount;
}

bool sg_heartTrail = false;
int  sg_heartTrailCount = 7;
static tConfItem<bool> cht( "MOD_HEART_TRAIL", sg_heartTrail );
static tConfItem<int>  chtc( "MOD_HEART_TRAIL_COUNT", sg_heartTrailCount );

bool rc_HeartTrail()
{
    return sg_heartTrail;
}

int rc_HeartTrailCount()
{
    if ( sg_heartTrailCount < 1 )  return 1;
    if ( sg_heartTrailCount > 24 ) return 24;
    return sg_heartTrailCount;
}

REAL sg_sparkRate = 18.0f;
static tConfItem<REAL> csr( "MOD_SPARK_RATE", sg_sparkRate );

bool rc_SparkDue( REAL now )
{
    static REAL last = -1000.0f;

    REAL rate = sg_sparkRate;
    if ( rate < 1.0f )  rate = 1.0f;
    if ( rate > 60.0f ) rate = 60.0f;

    REAL gap = 1.0f / rate;

    // Held across every cycle rather than one each: two riders grinding at
    // once is exactly when this matters, and it is the total on screen that
    // costs, not the total per player.
    if ( now < last )
        last = now - gap;   // a new round, and the clock went back

    if ( now - last < gap )
        return false;

    last = now;
    return true;
}

int rc_HeartSparkCount()
{
    if ( sg_heartSparkCount < 1 )  return 1;
    if ( sg_heartSparkCount > 40 ) return 40;
    return sg_heartSparkCount;
}

bool rc_HeartSparks()
{
    return sg_heartSparks;
}


void rc_HeartSolidInto( REAL x, REAL y, REAL z, REAL size, REAL turn, REAL alpha )
{
#ifndef DEDICATED
    if ( size <= 0 || alpha <= 0 )
        return;

    REAL const * hx;
    REAL const * hy;
    int steps = rc_HeartShape( hx, hy );

    // One colour, and it is pink. Taking the player's colour or the blast's
    // gave a mess of shades that matched nothing else on the screen.
    glColor4f( 1.00f, 0.24f, 0.52f, alpha );

    // The shape is a flat outline that swells through the middle: the centre
    // stands proud on both sides and the surface falls away from it to nothing
    // at the edge, so the rim stays sharp and the body is rounded. A slab with
    // a front, a back and a wall between them - which is what this was - reads
    // as a cut out rather than a heart.
    REAL const belly = size * 0.55f;
    REAL const shoulder = belly * 0.72f;   // how proud the halfway ring stands
    REAL const waist = 0.55f;              // where that ring sits, across

    REAL const spin = cosf( turn ), lean = sinf( turn );

    for ( int i = 0; i < steps; ++i )
    {
        // Closed: the last point joins the first. Left open, the seam falls at
        // the bottom of the heart and shows as a split down its point.
        int next = ( i + 1 ) % steps;

        REAL ax = hx[i] * size * 2.0f,    az = hy[i] * size * 2.0f;
        REAL bx = hx[next] * size * 2.0f, bz = hy[next] * size * 2.0f;

        REAL mx = ax * waist, mz = az * waist;
        REAL nx = bx * waist, nz = bz * waist;

        // Each side is two rings: middle to halfway, halfway to the edge.
        for ( int side = 0; side < 2; ++side )
        {
            REAL out = ( side == 0 ) ? 1.0f : -1.0f;

            REAL peak = belly * out;
            REAL mid  = shoulder * out;

            // middle -> halfway ring
            glVertex3f( x - peak * lean, y + peak * spin, z );
            glVertex3f( x + mx * spin - mid * lean, y + mx * lean + mid * spin, z + mz );
            glVertex3f( x + nx * spin - mid * lean, y + nx * lean + mid * spin, z + nz );

            // halfway ring -> the edge, which lies flat
            glVertex3f( x + mx * spin - mid * lean, y + mx * lean + mid * spin, z + mz );
            glVertex3f( x + ax * spin, y + ax * lean, z + az );
            glVertex3f( x + bx * spin, y + bx * lean, z + bz );

            glVertex3f( x + mx * spin - mid * lean, y + mx * lean + mid * spin, z + mz );
            glVertex3f( x + bx * spin, y + bx * lean, z + bz );
            glVertex3f( x + nx * spin - mid * lean, y + nx * lean + mid * spin, z + nz );
        }
    }

#endif
}

void rc_HeartSolid( REAL x, REAL y, REAL z, REAL size, REAL turn, REAL alpha )
{
#ifndef DEDICATED
    BeginTriangles();
    rc_HeartSolidInto( x, y, z, size, turn, alpha );
    RenderEnd();
#endif
}
