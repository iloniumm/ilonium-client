#include "rUpscale.h"

#include "tConfiguration.h"

#ifndef DEDICATED
#include <SDL3/SDL.h>
#endif

#include <math.h>
#include <string.h>
#include <vector>

// Two by default. It costs four times the texture memory on art that is
// mostly 256 pixels square, and a fraction of a second once at load; against
// that, every wall and every letter stops being a staircase.
static int sg_upscale = 2;
static tConfItem< int > sg_upscaleConf( "TEXTURE_UPSCALE", sg_upscale );

// Above this, a texture was authored for modern screens and enlarging it only
// spends memory. Doubling a 1024 wall gains nothing anybody can see.
static int sg_upscaleSourceMax = 512;
static tConfItem< int > sg_upscaleSourceMaxConf( "TEXTURE_UPSCALE_SOURCE_MAX", sg_upscaleSourceMax );

// A ceiling of our own, under the card's. Filling video memory with enlarged
// copies is the one way this feature can make the game worse.
static int sg_upscaleCeiling = 2048;
static tConfItem< int > sg_upscaleCeilingConf( "TEXTURE_UPSCALE_CEILING", sg_upscaleCeiling );

static int sg_upscaleMode = rcUpscale_Auto;
static tConfItem< int > sg_upscaleModeConf( "TEXTURE_UPSCALE_MODE", sg_upscaleMode );

static bool sg_upscaleDirty = false;

int rc_UpscaleFactorSetting(){ return sg_upscale; }
int rc_UpscaleModeSetting(){ return sg_upscaleMode; }

void rc_UpscaleFactorSet( int factor )
{
    if ( factor < 1 ) factor = 1;
    if ( factor > 4 ) factor = 4;
    if ( factor == sg_upscale ) return;

    sg_upscale = factor;
    sg_upscaleDirty = true;
}

void rc_UpscaleModeSet( int mode )
{
    if ( mode < 0 ) mode = 0;
    if ( mode > 2 ) mode = 2;
    if ( mode == sg_upscaleMode ) return;

    sg_upscaleMode = mode;
    sg_upscaleDirty = true;
}

bool rc_UpscaleDirty(){ return sg_upscaleDirty; }
void rc_UpscaleClean(){ sg_upscaleDirty = false; }

int rc_UpscaleFactor()
{
    if ( sg_upscale < 1 ) return 1;
    if ( sg_upscale > 4 ) return 4;
    return sg_upscale;
}

#ifndef DEDICATED

namespace
{

//! One pixel, however many bytes it happens to be.
struct Pel
{
    unsigned char b[ 4 ];
};

inline Pel Read( unsigned char const * base, int pitch, int depth, int x, int y )
{
    unsigned char const * p = base + (size_t)y * pitch + (size_t)x * depth;

    Pel out;
    out.b[0] = out.b[1] = out.b[2] = out.b[3] = 0;
    for ( int i = 0; i < depth; ++i )
        out.b[i] = p[i];

    return out;
}

inline void Write( unsigned char * base, int pitch, int depth, int x, int y, Pel const & v )
{
    unsigned char * p = base + (size_t)y * pitch + (size_t)x * depth;
    for ( int i = 0; i < depth; ++i )
        p[i] = v.b[i];
}

//! Clamped read, so the rules below can reach past the border without a special
//! case at every edge.
inline Pel At( unsigned char const * base, int pitch, int depth, int w, int h, int x, int y )
{
    if ( x < 0 ) x = 0;
    if ( y < 0 ) y = 0;
    if ( x >= w ) x = w - 1;
    if ( y >= h ) y = h - 1;
    return Read( base, pitch, depth, x, y );
}

//! Which channel holds opacity, or -1 when the surface has none.
inline int AlphaByte( int depth )
{
    if ( depth == 2 ) return 1;
    if ( depth == 4 ) return 3;
    return -1;
}

//! How far apart two pixels look, rather than how far apart their bytes are.
//!
//! Comparing bytes for equality is what the first version did, and it is why
//! anti-aliased art got no help at all: an edge the artist softened has no two
//! neighbouring pixels exactly alike, so every rule declined to fire. Weighted
//! towards brightness, because that is what the eye reads an edge by.
inline int Apart( Pel const & a, Pel const & b, int depth )
{
    if ( depth <= 2 )
    {
        int d = a.b[0] > b.b[0] ? a.b[0] - b.b[0] : b.b[0] - a.b[0];
        if ( depth == 2 )
        {
            int da = a.b[1] > b.b[1] ? a.b[1] - b.b[1] : b.b[1] - a.b[1];
            if ( da > d ) d = da;
        }
        return d;
    }

    int r = (int)a.b[0] - (int)b.b[0];
    int g = (int)a.b[1] - (int)b.b[1];
    int bl = (int)a.b[2] - (int)b.b[2];

    int y = ( r * 299 + g * 587 + bl * 114 ) / 1000;
    int u = ( -r * 169 - g * 331 + bl * 500 ) / 1000;
    int v = ( r * 500 - g * 419 - bl * 81 ) / 1000;

    if ( y < 0 ) y = -y;
    if ( u < 0 ) u = -u;
    if ( v < 0 ) v = -v;

    int d = y + u / 2 + v / 2;

    if ( depth == 4 )
    {
        int da = a.b[3] > b.b[3] ? a.b[3] - b.b[3] : b.b[3] - a.b[3];
        if ( da > d ) d = da;
    }

    return d;
}

//! Close enough to be treated as the same colour by the shape rules.
int const kAlike = 40;

inline bool Alike( Pel const & a, Pel const & b, int depth )
{
    return Apart( a, b, depth ) < kAlike;
}

inline Pel Mix( Pel const & a, Pel const & b, float t, int depth )
{
    Pel out;
    out.b[0] = out.b[1] = out.b[2] = out.b[3] = 0;

    for ( int i = 0; i < depth; ++i )
    {
        float v = a.b[i] + ( (float)b.b[i] - (float)a.b[i] ) * t;
        if ( v < 0 ) v = 0;
        if ( v > 255 ) v = 255;
        out.b[i] = (unsigned char)( v + 0.5f );
    }

    return out;
}

//! How far a corner should travel from the centre towards the pair of
//! neighbours that agreed about it.
//!
//! Hard art wants nearly the whole way: that is what turns a staircase into a
//! clean diagonal. Art the artist already softened wants less, because the
//! shades between the two colours are information and replacing them with a
//! copy throws it away. How firmly the two neighbours agree tells the two
//! cases apart on its own.
inline float CornerWeight( Pel const & one, Pel const & two, int depth )
{
    float apart = (float)Apart( one, two, depth ) / (float)kAlike;
    if ( apart > 1.0f ) apart = 1.0f;

    return 0.35f + 0.55f * ( 1.0f - apart );
}

// ------------------------------------------------------------ edge directed

//! A pixel becomes four, and each corner is filled from the two neighbours
//! that agree about it - blended rather than copied.
//!
//! The rule for which corner to fill is AdvMAME's. What is different here is
//! that "agree" means looks alike rather than is byte for byte identical, and
//! that the fill is a mix. Those two changes are the whole difference between
//! something that only works on hard-edged pixel art and something that also
//! works on art with soft edges - which most of this game's art has.
void EdgeDouble( unsigned char const * src, int spitch,
                 unsigned char * dst, int dpitch,
                 int depth, int w, int h )
{
    for ( int y = 0; y < h; ++y )
    {
        for ( int x = 0; x < w; ++x )
        {
            Pel e = Read( src, spitch, depth, x, y );
            Pel b = At( src, spitch, depth, w, h, x, y - 1 );
            Pel d = At( src, spitch, depth, w, h, x - 1, y );
            Pel f = At( src, spitch, depth, w, h, x + 1, y );
            Pel hh = At( src, spitch, depth, w, h, x, y + 1 );

            Pel e0 = e, e1 = e, e2 = e, e3 = e;

            if ( !Alike( b, hh, depth ) && !Alike( d, f, depth ) )
            {
                if ( Alike( d, b, depth ) )
                    e0 = Mix( e, Mix( d, b, 0.5f, depth ), CornerWeight( d, b, depth ), depth );
                if ( Alike( b, f, depth ) )
                    e1 = Mix( e, Mix( b, f, 0.5f, depth ), CornerWeight( b, f, depth ), depth );
                if ( Alike( d, hh, depth ) )
                    e2 = Mix( e, Mix( d, hh, 0.5f, depth ), CornerWeight( d, hh, depth ), depth );
                if ( Alike( hh, f, depth ) )
                    e3 = Mix( e, Mix( hh, f, 0.5f, depth ), CornerWeight( hh, f, depth ), depth );
            }

            Write( dst, dpitch, depth, x * 2,     y * 2,     e0 );
            Write( dst, dpitch, depth, x * 2 + 1, y * 2,     e1 );
            Write( dst, dpitch, depth, x * 2,     y * 2 + 1, e2 );
            Write( dst, dpitch, depth, x * 2 + 1, y * 2 + 1, e3 );
        }
    }
}

//! The same idea at three, where the middle of each edge can also be filled
//! and the centre always keeps the original.
void EdgeTriple( unsigned char const * src, int spitch,
                 unsigned char * dst, int dpitch,
                 int depth, int w, int h )
{
    for ( int y = 0; y < h; ++y )
    {
        for ( int x = 0; x < w; ++x )
        {
            Pel a = At( src, spitch, depth, w, h, x - 1, y - 1 );
            Pel b = At( src, spitch, depth, w, h, x,     y - 1 );
            Pel c = At( src, spitch, depth, w, h, x + 1, y - 1 );
            Pel d = At( src, spitch, depth, w, h, x - 1, y     );
            Pel e = Read( src, spitch, depth, x, y );
            Pel f = At( src, spitch, depth, w, h, x + 1, y     );
            Pel g = At( src, spitch, depth, w, h, x - 1, y + 1 );
            Pel hh = At( src, spitch, depth, w, h, x,     y + 1 );
            Pel i = At( src, spitch, depth, w, h, x + 1, y + 1 );

            Pel o[ 9 ] = { e, e, e, e, e, e, e, e, e };

            if ( !Alike( b, hh, depth ) && !Alike( d, f, depth ) )
            {
                bool db = Alike( d, b, depth ), bf = Alike( b, f, depth );
                bool dh = Alike( d, hh, depth ), hf = Alike( hh, f, depth );

                if ( db ) o[0] = Mix( e, Mix( d, b, 0.5f, depth ), CornerWeight( d, b, depth ), depth );
                if ( bf ) o[2] = Mix( e, Mix( b, f, 0.5f, depth ), CornerWeight( b, f, depth ), depth );
                if ( dh ) o[6] = Mix( e, Mix( d, hh, 0.5f, depth ), CornerWeight( d, hh, depth ), depth );
                if ( hf ) o[8] = Mix( e, Mix( hh, f, 0.5f, depth ), CornerWeight( hh, f, depth ), depth );

                if ( ( db && !Alike( e, c, depth ) ) || ( bf && !Alike( e, a, depth ) ) )
                    o[1] = Mix( e, b, 0.5f, depth );
                if ( ( db && !Alike( e, g, depth ) ) || ( dh && !Alike( e, a, depth ) ) )
                    o[3] = Mix( e, d, 0.5f, depth );
                if ( ( bf && !Alike( e, i, depth ) ) || ( hf && !Alike( e, c, depth ) ) )
                    o[5] = Mix( e, f, 0.5f, depth );
                if ( ( hf && !Alike( e, g, depth ) ) || ( dh && !Alike( e, i, depth ) ) )
                    o[7] = Mix( e, hh, 0.5f, depth );
            }

            for ( int k = 0; k < 9; ++k )
                Write( dst, dpitch, depth, x * 3 + ( k % 3 ), y * 3 + ( k / 3 ), o[k] );
        }
    }
}

// -------------------------------------------------------------- windowed sinc

double Sinc( double x )
{
    if ( x < 1e-8 && x > -1e-8 )
        return 1.0;
    double p = M_PI * x;
    return sin( p ) / p;
}

//! Lanczos with three lobes: sharp enough that an enlarged floor still has
//! grain, soft enough that it does not lose its texture on the way there.
double Lanczos( double x )
{
    if ( x < 0 ) x = -x;
    if ( x >= 3.0 ) return 0.0;
    return Sinc( x ) * Sinc( x / 3.0 );
}

//! One axis worth of taps, worked out once and used for every row or column.
struct Taps
{
    std::vector< int >   first;   //!< first source sample for each output
    std::vector< float > weight;  //!< span weights, run together
    int span;
};

Taps Plan( int from, int to )
{
    // Enlarging, so the filter keeps its own width: three lobes each side.
    double const support = 3.0;
    double const scale = (double)from / (double)to;

    Taps t;
    t.span = (int)ceil( support * 2.0 ) + 2;
    t.first.resize( to );
    t.weight.resize( (size_t)to * t.span );

    for ( int o = 0; o < to; ++o )
    {
        double centre = ( o + 0.5 ) * scale - 0.5;
        int    start = (int)floor( centre - support ) + 1;

        t.first[ o ] = start;

        double sum = 0;
        for ( int k = 0; k < t.span; ++k )
        {
            double w = Lanczos( centre - (double)( start + k ) );
            t.weight[ (size_t)o * t.span + k ] = (float)w;
            sum += w;
        }

        // Normalised so flat areas keep their exact colour. Without this the
        // clamped edges drift a shade and every texture gets a faint frame.
        if ( sum > 1e-9 )
            for ( int k = 0; k < t.span; ++k )
                t.weight[ (size_t)o * t.span + k ] = (float)( t.weight[ (size_t)o * t.span + k ] / sum );
    }

    return t;
}

//! Enlarge by a windowed sinc, with opacity folded in and overshoot refused.
//!
//! Two things this filter does badly if left alone. It averages colour across
//! opacity, so the invisible pixels around a sprite - usually black - drag the
//! edge towards them and leave a dark rim; multiplying colour by opacity first
//! and dividing it back out at the end keeps them from voting.
//!
//! And its side lobes overshoot at a hard edge, laying a bright line along one
//! side and a dark line along the other. That halo is what a windowed sinc is
//! known for and what somebody looking at an enlarged logo will point at
//! first. The cure is to refuse it: no output sample may leave the range of
//! the samples that produced it. Edges keep their snap, the ringing has
//! nowhere to go.
void Resample( unsigned char const * src, int spitch,
               unsigned char * dst, int dpitch,
               int depth, int w, int h, int ow, int oh )
{
    Taps across = Plan( w, ow );
    Taps down = Plan( h, oh );

    int alpha = AlphaByte( depth );

    std::vector< float > mid( (size_t)ow * h * depth );

    for ( int y = 0; y < h; ++y )
    {
        for ( int o = 0; o < ow; ++o )
        {
            float acc[ 4 ] = { 0, 0, 0, 0 };
            float lo[ 4 ], hi[ 4 ];
            bool first = true;

            int start = across.first[ o ];
            float const * ws = &across.weight[ (size_t)o * across.span ];

            for ( int k = 0; k < across.span; ++k )
            {
                int sx = start + k;
                if ( sx < 0 ) sx = 0;
                if ( sx >= w ) sx = w - 1;

                float weight = ws[ k ];
                unsigned char const * p = src + (size_t)y * spitch + (size_t)sx * depth;

                float cover = ( alpha >= 0 ) ? p[ alpha ] * ( 1.0f / 255.0f ) : 1.0f;

                // Only the taps that carry weight define the range. A tap the
                // filter gave nothing to has no say in what the answer is
                // allowed to be.
                bool counts = ( weight > 1e-6f );

                for ( int c = 0; c < depth; ++c )
                {
                    float sample = p[c] * ( c == alpha ? 1.0f : cover );
                    acc[c] += weight * sample;

                    if ( counts )
                    {
                        if ( first || sample < lo[c] ) lo[c] = sample;
                        if ( first || sample > hi[c] ) hi[c] = sample;
                    }
                }

                if ( counts )
                    first = false;
            }

            float * out = &mid[ ( (size_t)y * ow + o ) * depth ];
            for ( int c = 0; c < depth; ++c )
            {
                float v = acc[c];
                if ( !first )
                {
                    if ( v < lo[c] ) v = lo[c];
                    if ( v > hi[c] ) v = hi[c];
                }
                out[c] = v;
            }
        }
    }

    for ( int o = 0; o < oh; ++o )
    {
        int start = down.first[ o ];

        for ( int x = 0; x < ow; ++x )
        {
            float acc[ 4 ] = { 0, 0, 0, 0 };
            float lo[ 4 ], hi[ 4 ];
            bool first = true;

            float const * ws = &down.weight[ (size_t)o * down.span ];

            for ( int k = 0; k < down.span; ++k )
            {
                int sy = start + k;
                if ( sy < 0 ) sy = 0;
                if ( sy >= h ) sy = h - 1;

                float weight = ws[ k ];
                float const * p = &mid[ ( (size_t)sy * ow + x ) * depth ];

                bool counts = ( weight > 1e-6f );

                for ( int c = 0; c < depth; ++c )
                {
                    acc[c] += weight * p[c];

                    if ( counts )
                    {
                        if ( first || p[c] < lo[c] ) lo[c] = p[c];
                        if ( first || p[c] > hi[c] ) hi[c] = p[c];
                    }
                }

                if ( counts )
                    first = false;
            }

            unsigned char * out = dst + (size_t)o * dpitch + (size_t)x * depth;

            float straight[ 4 ];
            for ( int c = 0; c < depth; ++c )
            {
                float v = acc[c];
                if ( !first )
                {
                    if ( v < lo[c] ) v = lo[c];
                    if ( v > hi[c] ) v = hi[c];
                }
                straight[c] = v;
            }

            float cover = ( alpha >= 0 ) ? straight[ alpha ] * ( 1.0f / 255.0f ) : 1.0f;
            if ( cover < 1.0f / 512.0f )
                cover = 1.0f / 512.0f;

            for ( int c = 0; c < depth; ++c )
            {
                float v = ( c == alpha ) ? straight[c] : straight[c] / cover;
                if ( v < 0 ) v = 0;
                if ( v > 255 ) v = 255;
                out[c] = (unsigned char)( v + 0.5 );
            }
        }
    }
}

// ---------------------------------------------------------------- measuring

// Eight, not sixteen. A logo sixty pixels across is four tiles wide at
// sixteen, and averaging with its neighbours then drowns it in whatever
// surrounds it - which is exactly the case this was written for.
int const kTile = 8;

//! How drawn each part of the picture looks, one number per tile.
//!
//! One number for the whole file was the first version's mistake, and it is
//! exactly what somebody noticed: the arena rim is a hard-edged logo sitting
//! on a field of noise, the noise won the vote, and the logo went through the
//! filter meant for photographs. Measured in tiles, the logo and the noise
//! each get the treatment they want.
//!
//! The reading is the share of neighbouring pairs that are exactly the same
//! colour. Drawn art is built out of filled areas, so most of its neighbours
//! match to the byte. Painted art almost never repeats itself, and neither
//! does noise - which matters, because noise is the one thing that fools the
//! obvious alternative: counting small differences calls a noise field
//! "drawn", since noise jumps rather than drifts.
//!
//! Across the shipped art the two groups sit far apart: the fonts, the floor,
//! the arrow wall and the cycle body run from three quarters to all, and the
//! rim, the sky, the flames and the logo screen all come in under two fifths.
struct Drawnness
{
    std::vector< float > v;
    int w, h;

    float At( int tx, int ty ) const
    {
        if ( tx < 0 ) tx = 0;
        if ( ty < 0 ) ty = 0;
        if ( tx >= w ) tx = w - 1;
        if ( ty >= h ) ty = h - 1;
        return v[ (size_t)ty * w + tx ];
    }

    //! Read at a source pixel, smoothly, so neighbouring tiles do not leave a
    //! seam where they disagree.
    float Sample( float px, float py ) const
    {
        float fx = px / kTile - 0.5f;
        float fy = py / kTile - 0.5f;

        int x0 = (int)floor( fx ), y0 = (int)floor( fy );
        float sx = fx - x0, sy = fy - y0;

        float a = At( x0, y0 ) * ( 1 - sx ) + At( x0 + 1, y0 ) * sx;
        float b = At( x0, y0 + 1 ) * ( 1 - sx ) + At( x0 + 1, y0 + 1 ) * sx;

        return a * ( 1 - sy ) + b * sy;
    }
};

Drawnness Measure( unsigned char const * base, int pitch, int depth, int w, int h )
{
    int alpha = AlphaByte( depth );
    int colours = ( depth >= 3 ) ? 3 : 1;

    Drawnness map;
    map.w = ( w + kTile - 1 ) / kTile;
    map.h = ( h + kTile - 1 ) / kTile;
    map.v.assign( (size_t)map.w * map.h, 1.0f );

    // The gap the shipped art leaves between the two kinds. Anything landing
    // inside it is genuinely mixed and gets a blend of both treatments rather
    // than a coin toss.
    float const soft = 0.45f, hard = 0.62f;

    std::vector< float > raw( (size_t)map.w * map.h, 1.0f );

    // Not every tile has an opinion. One that is wholly transparent, or almost
    // so, has nothing to measure; letting it default either way and then
    // averaging it into its neighbours is how a soft decal ends up with hard
    // edges around its fringe.
    std::vector< float > sure( (size_t)map.w * map.h, 0.0f );

    long allCounted = 0, allFlat = 0;

    for ( int ty = 0; ty < map.h; ++ty )
    {
        for ( int tx = 0; tx < map.w; ++tx )
        {
            long counted = 0, flat = 0;

            int x1 = tx * kTile + kTile; if ( x1 > w - 1 ) x1 = w - 1;
            int y1 = ty * kTile + kTile; if ( y1 > h - 1 ) y1 = h - 1;

            for ( int y = ty * kTile; y < y1; ++y )
            {
                for ( int x = tx * kTile; x < x1; ++x )
                {
                    Pel a0 = Read( base, pitch, depth, x, y );

                    if ( alpha >= 0 && a0.b[ alpha ] <= 8 )
                        continue;

                    Pel side[ 2 ] = { Read( base, pitch, depth, x + 1, y ),
                                      Read( base, pitch, depth, x, y + 1 ) };

                    for ( int k = 0; k < 2; ++k )
                    {
                        if ( alpha >= 0 && side[k].b[ alpha ] <= 8 )
                            continue;

                        int worst = 0;
                        for ( int c = 0; c < colours; ++c )
                        {
                            int d = (int)a0.b[c] - (int)side[k].b[c];
                            if ( d < 0 ) d = -d;
                            if ( d > worst ) worst = d;
                        }

                        ++counted;
                        if ( worst == 0 )
                            ++flat;
                    }
                }
            }

            allCounted += counted;
            allFlat += flat;

            if ( counted >= 16 )
            {
                float share = (float)flat / (float)counted;
                float drawn;

                if ( share >= hard )      drawn = 1.0f;
                else if ( share <= soft ) drawn = 0.0f;
                else                      drawn = ( share - soft ) / ( hard - soft );

                raw[ (size_t)ty * map.w + tx ] = drawn;
                sure[ (size_t)ty * map.w + tx ] = 1.0f;
            }
        }
    }

    // What the picture says about itself, for the tiles that had nothing to
    // say and no neighbour to borrow from.
    float overall = 0.0f;
    if ( allCounted >= 64 )
    {
        float share = (float)allFlat / (float)allCounted;
        overall = share >= hard ? 1.0f : ( share <= soft ? 0.0f : ( share - soft ) / ( hard - soft ) );
    }

    // A tile is a small sample and its reading is noisy. Averaging with its
    // neighbours settles it - but only with the neighbours that had something
    // to measure.
    for ( int ty = 0; ty < map.h; ++ty )
    {
        for ( int tx = 0; tx < map.w; ++tx )
        {
            float sum = 0, weight = 0;

            for ( int dy = -1; dy <= 1; ++dy )
            {
                for ( int dx = -1; dx <= 1; ++dx )
                {
                    int sx = tx + dx, sy = ty + dy;
                    if ( sx < 0 || sy < 0 || sx >= map.w || sy >= map.h ) continue;

                    size_t at = (size_t)sy * map.w + sx;
                    if ( sure[ at ] <= 0.0f ) continue;

                    // Its own reading counts for more than its neighbours'.
                    float k = ( dx == 0 && dy == 0 ) ? 4.0f : 1.0f;
                    sum += raw[ at ] * k;
                    weight += k;
                }
            }

            map.v[ (size_t)ty * map.w + tx ] = weight > 0 ? sum / weight : overall;
        }
    }

    return map;
}

}

SDL_Surface * rc_UpscaleSurface( SDL_Surface * from, int ceiling )
{
    int factor = rc_UpscaleFactor();
    if ( factor < 2 || !from )
        return 0;

    int w = from->w, h = from->h;
    if ( w < 2 || h < 2 )
        return 0;

    int depth = SDL_BYTESPERPIXEL( from->format );
    if ( depth < 1 || depth > 4 )
        return 0;

    int longest = w > h ? w : h;
    if ( longest > sg_upscaleSourceMax )
        return 0;

    // Trim the factor until the result fits both our ceiling and the card's,
    // rather than refusing outright: a texture that can only double should
    // still double.
    int limit = sg_upscaleCeiling;
    if ( ceiling > 0 && ceiling < limit )
        limit = ceiling;

    while ( factor > 1 && longest * factor > limit )
        --factor;

    if ( factor < 2 )
        return 0;

    SDL_Surface * out = SDL_CreateSurface( w * factor, h * factor, from->format );
    if ( !out )
        return 0;

    bool lockFrom = SDL_MUSTLOCK( from );
    bool lockOut = SDL_MUSTLOCK( out );

    if ( lockFrom && !SDL_LockSurface( from ) )
    {
        SDL_DestroySurface( out );
        return 0;
    }
    if ( lockOut && !SDL_LockSurface( out ) )
    {
        if ( lockFrom ) SDL_UnlockSurface( from );
        SDL_DestroySurface( out );
        return 0;
    }

    unsigned char const * src = (unsigned char const *)from->pixels;
    unsigned char * dst = (unsigned char *)out->pixels;

    int ow = w * factor, oh = h * factor;

    // How much of the picture wants which treatment. Forced modes skip the
    // measuring; auto looks, and usually finds the whole image agrees with
    // itself anyway - in which case only one of the two ever runs.
    Drawnness map;
    bool wantEdge = false, wantSmooth = false;

    if ( sg_upscaleMode == rcUpscale_Sharp )
    {
        wantEdge = true;
    }
    else if ( sg_upscaleMode == rcUpscale_Smooth )
    {
        wantSmooth = true;
    }
    else
    {
        map = Measure( src, from->pitch, depth, w, h );

        for ( size_t i = 0; i < map.v.size(); ++i )
        {
            if ( map.v[i] > 0.001f ) wantEdge = true;
            if ( map.v[i] < 0.999f ) wantSmooth = true;
        }
    }

    // Only the tripling has a rule of its own. Four goes through the doubling
    // twice, so the second pass carries on from the diagonals the first one
    // built; a direct quadrupling cannot see them.
    std::vector< unsigned char > edge, smooth;
    int stride = ow * depth;

    if ( wantEdge )
    {
        edge.resize( (size_t)stride * oh );

        if ( factor == 3 )
        {
            EdgeTriple( src, from->pitch, &edge[0], stride, depth, w, h );
        }
        else
        {
            if ( factor == 2 )
            {
                EdgeDouble( src, from->pitch, &edge[0], stride, depth, w, h );
            }
            else
            {
                std::vector< unsigned char > once( (size_t)w * 2 * depth * h * 2 );
                int oncePitch = w * 2 * depth;
                EdgeDouble( src, from->pitch, &once[0], oncePitch, depth, w, h );
                EdgeDouble( &once[0], oncePitch, &edge[0], stride, depth, w * 2, h * 2 );
            }
        }
    }

    if ( wantSmooth )
    {
        smooth.resize( (size_t)stride * oh );
        Resample( src, from->pitch, &smooth[0], stride, depth, w, h, ow, oh );
    }

    for ( int y = 0; y < oh; ++y )
    {
        for ( int x = 0; x < ow; ++x )
        {
            unsigned char * o = dst + (size_t)y * out->pitch + (size_t)x * depth;

            if ( !wantSmooth )
            {
                memcpy( o, &edge[ (size_t)y * stride + (size_t)x * depth ], depth );
                continue;
            }
            if ( !wantEdge )
            {
                memcpy( o, &smooth[ (size_t)y * stride + (size_t)x * depth ], depth );
                continue;
            }

            float t = map.Sample( ( x + 0.5f ) / factor, ( y + 0.5f ) / factor );
            if ( t < 0 ) t = 0;
            if ( t > 1 ) t = 1;

            unsigned char const * a = &smooth[ (size_t)y * stride + (size_t)x * depth ];
            unsigned char const * b = &edge[ (size_t)y * stride + (size_t)x * depth ];

            for ( int c = 0; c < depth; ++c )
            {
                float v = a[c] + ( (float)b[c] - (float)a[c] ) * t;
                if ( v < 0 ) v = 0;
                if ( v > 255 ) v = 255;
                o[c] = (unsigned char)( v + 0.5f );
            }
        }
    }

    if ( lockOut ) SDL_UnlockSurface( out );
    if ( lockFrom ) SDL_UnlockSurface( from );

    return out;
}

#else

SDL_Surface * rc_UpscaleSurface( SDL_Surface *, int )
{
    return 0;
}

#endif
