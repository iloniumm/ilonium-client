#include "ModTheme.h"

#include <map>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <deque>
#include <string>
#include "eSound.h"
#include "tSysTime.h"

namespace
{

//! Written the way they are read everywhere else, 0xRRGGBB, and turned into
//! what the drawing code wants here rather than at every use.
ImU32 Hex( unsigned int rgb, float alpha = 1.0f )
{
    return IM_COL32( ( rgb >> 16 ) & 0xFF,
                     ( rgb >> 8 )  & 0xFF,
                     rgb           & 0xFF,
                     (int)( alpha * 255.0f + 0.5f ) );
}

rcAccent MakeAccent( unsigned int solid, unsigned int bright )
{
    rcAccent a;
    a.solid  = Hex( solid );
    a.bright = Hex( bright );
    a.line   = Hex( solid, 0.55f );
    a.wash   = Hex( solid, 0.14f );
    return a;
}

rcTheme Build()
{
    rcTheme t;

    // Six steps, and the gap between them is small on purpose. A ladder that
    // is easy to see is a ladder that looks like stripes.
    t.page      = Hex( 0x07060B );
    t.sunken    = Hex( 0x0A0810 );
    t.surface   = Hex( 0x0E0B15 );
    t.surfaceHi = Hex( 0x13101C );
    t.raised    = Hex( 0x191524 );
    t.raisedHi  = Hex( 0x201B2E );

    // Tinted rather than grey, so an outline never looks like a scratch on
    // the screen. Both are far below the strength one would guess.
    t.line       = Hex( 0x8C7FA8, 0.12f );
    t.lineStrong = Hex( 0x8C7FA8, 0.28f );

    // Never pure white: at this contrast it buzzes.
    t.text     = Hex( 0xF2EFF7 );
    t.textDim  = Hex( 0xA79FB8 );
    t.textMute = Hex( 0x6E6780 );

    // The identity stays the rose it already was, taken off full saturation
    // where it was shouting. Cyan carries anything alive - a rate, a ping, a
    // track playing. Gold is rationed: records and first place, nothing else.
    t.primary = MakeAccent( 0xFF3D7F, 0xFF8FB4 );
    t.second  = MakeAccent( 0x3EE8FF, 0xA6F4FF );
    t.gold    = MakeAccent( 0xFFC75A, 0xFFE097 );

    t.good = Hex( 0x3DFF9E );
    t.bad  = Hex( 0xFF6B7F );
    t.warn = Hex( 0xFF9A3D );

    t.radiusSmall  = 6.0f;
    t.radiusMedium = 10.0f;
    t.radiusLarge  = 16.0f;
    t.radiusPill   = 999.0f;

    t.unit = 4.0f;

    t.quick  = 0.14f;
    t.settle = 0.16f;

    return t;
}

//! Where each widget has got to. Keyed by the widget's own id, so nothing has
//! to be threaded through the drawing code to make an animation work.
std::map< ImGuiID, float > & Progress()
{
    static std::map< ImGuiID, float > all;
    return all;
}

}

float rc_PageEase( int token )
{
    static std::map< int, float > arrived;
    static int showing = -999999;

    float now = (float)ImGui::GetTime();

    if ( token != showing )
    {
        showing = token;
        if ( arrived.find( token ) == arrived.end() || now - arrived[ token ] > rc_Theme().settle )
            arrived[ token ] = now;

        // one page at a time is enough to remember
        if ( arrived.size() > 64 )
            arrived.clear();
    }

    float t = ( now - arrived[ token ] ) / rc_Theme().settle;
    if ( t >= 1.0f )
        return 1.0f;
    if ( t <= 0.0f )
        return 0.0f;

    // Fast at first and easing out, which is how things that have weight
    // arrive; the other way round reads as a machine.
    return 1.0f - ( 1.0f - t ) * ( 1.0f - t ) * ( 1.0f - t );
}

rcTheme const & rc_Theme()
{
    static rcTheme const theme = Build();
    return theme;
}

ImU32 rc_Fade( ImU32 colour, float alpha )
{
    ImVec4 c = ImGui::ColorConvertU32ToFloat4( colour );
    c.w *= alpha;
    return ImGui::ColorConvertFloat4ToU32( c );
}

ImU32 rc_Mix( ImU32 from, ImU32 to, float t )
{
    if ( t <= 0.0f ) return from;
    if ( t >= 1.0f ) return to;

    ImVec4 a = ImGui::ColorConvertU32ToFloat4( from );
    ImVec4 b = ImGui::ColorConvertU32ToFloat4( to );

    return ImGui::ColorConvertFloat4ToU32(
        ImVec4( a.x + ( b.x - a.x ) * t,
                a.y + ( b.y - a.y ) * t,
                a.z + ( b.z - a.z ) * t,
                a.w + ( b.w - a.w ) * t ) );
}

float rc_Ease( ImGuiID id, float toward, float speed )
{
    rcTheme const & t = rc_Theme();
    if ( speed <= 0.0f )
        speed = t.quick;

    float & at = Progress()[ id ];

    // Framerate independent, and this game runs anywhere from sixty to well
    // past a thousand: a fixed step per frame would make the same animation
    // take a fifth of the time on a fast machine.
    float dt = ImGui::GetIO().DeltaTime;
    if ( dt <= 0.0f )
        return at;
    if ( dt > 0.1f )
        dt = 0.1f;

    float rate = 1.0f - expf( -dt / ( speed * 0.35f ) );
    at += ( toward - at ) * rate;

    if ( fabsf( toward - at ) < 0.001f )
        at = toward;

    return at;
}

float rc_TrackedWidth( char const * text, float tracking )
{
    if ( !text || !*text )
        return 0.0f;

    float width = 0.0f;
    for ( char const * c = text; *c; ++c )
        width += ImGui::CalcTextSize( c, c + 1 ).x + tracking;

    return width - tracking;
}

void rc_Tracked( char const * text, float tracking, ImU32 colour )
{
    if ( !text || !*text )
        return;

    ImDrawList * dl = ImGui::GetWindowDrawList();
    ImVec2 at = ImGui::GetCursorScreenPos();
    float start = at.x;

    for ( char const * c = text; *c; ++c )
    {
        dl->AddText( at, colour, c, c + 1 );
        at.x += ImGui::CalcTextSize( c, c + 1 ).x + tracking;
    }

    ImGui::Dummy( ImVec2( at.x - start - tracking, ImGui::GetTextLineHeight() ) );
}

void rc_ApplyStyle( ImGuiStyle & style, float scale )
{
    rcTheme const & t = rc_Theme();

    if ( scale <= 0.0f )
        scale = 1.0f;

    float u = t.unit * scale;

    // Every gap is a multiple of one number. This is the whole of what people
    // mean when they say an interface looks tidy.
    style.WindowPadding     = ImVec2( u * 5, u * 5 );
    style.FramePadding      = ImVec2( u * 3, u * 2.5f );
    style.CellPadding       = ImVec2( u * 3, u * 2 );
    style.ItemSpacing       = ImVec2( u * 3, u * 2.5f );
    style.ItemInnerSpacing  = ImVec2( u * 2, u * 2 );
    style.IndentSpacing     = u * 6;
    style.ScrollbarSize     = u * 2.5f;
    style.GrabMinSize       = u * 3;

    style.WindowRounding    = t.radiusLarge * scale;
    style.ChildRounding     = t.radiusMedium * scale;
    style.FrameRounding     = t.radiusSmall * scale;
    style.PopupRounding     = t.radiusMedium * scale;
    style.GrabRounding      = t.radiusPill;
    style.ScrollbarRounding = t.radiusPill;
    style.TabRounding       = t.radiusSmall * scale;

    // One pixel, and only because the hairline colour is faint enough to earn
    // it. Thicker than this and the ladder stops doing the work.
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize  = 1.0f;
    style.FrameBorderSize  = 1.0f;
    style.PopupBorderSize  = 1.0f;
    style.TabBorderSize    = 0.0f;

    style.WindowTitleAlign = ImVec2( 0.0f, 0.5f );
    style.ButtonTextAlign  = ImVec2( 0.5f, 0.5f );

    style.AntiAliasedLines       = true;
    style.AntiAliasedLinesUseTex = true;
    style.AntiAliasedFill        = true;

    ImVec4 * c = style.Colors;

    c[ ImGuiCol_Text ]                 = ImGui::ColorConvertU32ToFloat4( t.text );
    c[ ImGuiCol_TextDisabled ]         = ImGui::ColorConvertU32ToFloat4( t.textMute );

    c[ ImGuiCol_WindowBg ]             = ImGui::ColorConvertU32ToFloat4( t.page );
    c[ ImGuiCol_ChildBg ]              = ImGui::ColorConvertU32ToFloat4( t.surface );
    c[ ImGuiCol_PopupBg ]              = ImGui::ColorConvertU32ToFloat4( t.raised );

    c[ ImGuiCol_Border ]               = ImGui::ColorConvertU32ToFloat4( t.line );
    c[ ImGuiCol_BorderShadow ]         = ImVec4( 0, 0, 0, 0 );

    // A control at rest sits one step up from what it is on, not in a box of
    // its own colour.
    c[ ImGuiCol_FrameBg ]              = ImGui::ColorConvertU32ToFloat4( t.surfaceHi );
    c[ ImGuiCol_FrameBgHovered ]       = ImGui::ColorConvertU32ToFloat4( t.raised );
    c[ ImGuiCol_FrameBgActive ]        = ImGui::ColorConvertU32ToFloat4( t.raisedHi );

    c[ ImGuiCol_TitleBg ]              = ImGui::ColorConvertU32ToFloat4( t.sunken );
    c[ ImGuiCol_TitleBgActive ]        = ImGui::ColorConvertU32ToFloat4( t.sunken );
    c[ ImGuiCol_TitleBgCollapsed ]     = ImGui::ColorConvertU32ToFloat4( t.page );

    c[ ImGuiCol_MenuBarBg ]            = ImGui::ColorConvertU32ToFloat4( t.sunken );

    c[ ImGuiCol_ScrollbarBg ]          = ImVec4( 0, 0, 0, 0 );
    c[ ImGuiCol_ScrollbarGrab ]        = ImGui::ColorConvertU32ToFloat4( t.lineStrong );
    c[ ImGuiCol_ScrollbarGrabHovered ] = ImGui::ColorConvertU32ToFloat4( rc_Fade( t.primary.solid, 0.5f ) );
    c[ ImGuiCol_ScrollbarGrabActive ]  = ImGui::ColorConvertU32ToFloat4( t.primary.solid );

    c[ ImGuiCol_CheckMark ]            = ImGui::ColorConvertU32ToFloat4( t.primary.solid );
    c[ ImGuiCol_SliderGrab ]           = ImGui::ColorConvertU32ToFloat4( t.primary.solid );
    c[ ImGuiCol_SliderGrabActive ]     = ImGui::ColorConvertU32ToFloat4( t.primary.bright );

    // The wash, never the solid. A button filled with the identity colour is
    // the single loudest thing an interface can do, and there should be at
    // most one of those on screen at a time.
    c[ ImGuiCol_Button ]               = ImGui::ColorConvertU32ToFloat4( t.surfaceHi );
    c[ ImGuiCol_ButtonHovered ]        = ImGui::ColorConvertU32ToFloat4( t.raised );
    c[ ImGuiCol_ButtonActive ]         = ImGui::ColorConvertU32ToFloat4( t.primary.wash );

    c[ ImGuiCol_Header ]               = ImGui::ColorConvertU32ToFloat4( t.primary.wash );
    c[ ImGuiCol_HeaderHovered ]        = ImGui::ColorConvertU32ToFloat4( rc_Fade( t.primary.solid, 0.20f ) );
    c[ ImGuiCol_HeaderActive ]         = ImGui::ColorConvertU32ToFloat4( rc_Fade( t.primary.solid, 0.26f ) );

    c[ ImGuiCol_Separator ]            = ImGui::ColorConvertU32ToFloat4( t.line );
    c[ ImGuiCol_SeparatorHovered ]     = ImGui::ColorConvertU32ToFloat4( t.lineStrong );
    c[ ImGuiCol_SeparatorActive ]      = ImGui::ColorConvertU32ToFloat4( t.primary.line );

    c[ ImGuiCol_ResizeGrip ]           = ImGui::ColorConvertU32ToFloat4( t.line );
    c[ ImGuiCol_ResizeGripHovered ]    = ImGui::ColorConvertU32ToFloat4( t.primary.line );
    c[ ImGuiCol_ResizeGripActive ]     = ImGui::ColorConvertU32ToFloat4( t.primary.solid );

    c[ ImGuiCol_Tab ]                  = ImGui::ColorConvertU32ToFloat4( t.sunken );
    c[ ImGuiCol_TabHovered ]           = ImGui::ColorConvertU32ToFloat4( t.primary.wash );
    c[ ImGuiCol_TabSelected ]          = ImGui::ColorConvertU32ToFloat4( t.surface );
    c[ ImGuiCol_TabDimmed ]            = ImGui::ColorConvertU32ToFloat4( t.page );
    c[ ImGuiCol_TabDimmedSelected ]    = ImGui::ColorConvertU32ToFloat4( t.surface );

    c[ ImGuiCol_PlotLines ]            = ImGui::ColorConvertU32ToFloat4( t.second.solid );
    c[ ImGuiCol_PlotLinesHovered ]     = ImGui::ColorConvertU32ToFloat4( t.second.bright );
    c[ ImGuiCol_PlotHistogram ]        = ImGui::ColorConvertU32ToFloat4( t.primary.solid );
    c[ ImGuiCol_PlotHistogramHovered ] = ImGui::ColorConvertU32ToFloat4( t.primary.bright );

    c[ ImGuiCol_TableHeaderBg ]        = ImGui::ColorConvertU32ToFloat4( t.sunken );
    c[ ImGuiCol_TableBorderStrong ]    = ImGui::ColorConvertU32ToFloat4( t.lineStrong );
    c[ ImGuiCol_TableBorderLight ]     = ImGui::ColorConvertU32ToFloat4( t.line );
    c[ ImGuiCol_TableRowBg ]           = ImVec4( 0, 0, 0, 0 );
    c[ ImGuiCol_TableRowBgAlt ]        = ImGui::ColorConvertU32ToFloat4( rc_Fade( t.surfaceHi, 0.5f ) );

    c[ ImGuiCol_TextSelectedBg ]       = ImGui::ColorConvertU32ToFloat4( t.primary.wash );
    c[ ImGuiCol_DragDropTarget ]       = ImGui::ColorConvertU32ToFloat4( t.primary.solid );
    c[ ImGuiCol_NavCursor ]            = ImGui::ColorConvertU32ToFloat4( t.primary.line );

    // What sits behind a modal. Dark enough to remove the page from the
    // conversation, not so dark that it disappears.
    c[ ImGuiCol_ModalWindowDimBg ]     = ImVec4( 0.02f, 0.01f, 0.04f, 0.72f );
}

void rc_DrawHeart( ImDrawList * dl, ImVec2 at, float size, float lean, ImU32 colour )
{
    if ( !dl || size <= 0.0f )
        return;

    // The usual curve for this shape. Twenty six steps is where the outline
    // stops looking like a polygon at the sizes these are drawn at, and every
    // step past that is spent on nothing.
    int const steps = 26;

    // The curve reaches sixteen across and about seventeen down; scaling by
    // the taller of the two keeps the size argument meaning what it says.
    float const scale = size / 17.0f;
    float const turn = sinf( lean ), lay = cosf( lean );

    dl->PathClear();
    for ( int i = 0; i < steps; ++i )
    {
        float t = ( (float)i / (float)steps ) * 6.2831853f;
        float st = sinf( t );

        float x = 16.0f * st * st * st;
        float y = -( 13.0f * cosf( t ) - 5.0f * cosf( 2.0f * t )
                     - 2.0f * cosf( 3.0f * t ) - cosf( 4.0f * t ) );

        x *= scale;
        y *= scale;

        dl->PathLineTo( ImVec2( at.x + x * lay - y * turn,
                                at.y + x * turn + y * lay ) );
    }

    // Concave, so it cannot be filled as a fan from one corner the way the
    // simpler shapes are - the notch at the top would be filled in too.
    dl->PathFillConcave( colour );
}

void rc_DrawBackdrop( ImDrawList * dl, ImVec2 at, ImVec2 size, float alpha )
{
    if ( !dl || alpha <= 0.01f || size.x < 8.0f || size.y < 8.0f )
        return;

    rcTheme const & th = rc_Theme();
    ImVec2 bot( at.x + size.x, at.y + size.y );

    float time = (float)ImGui::GetTime();

    // Where the floor begins. Well below the middle, so the panels sit in the
    // sky rather than on the ground and the grid stays a suggestion.
    float horizon = at.y + size.y * 0.58f;

    // The sky: the page colour, lifting a shade towards the horizon.
    dl->AddRectFilledMultiColor( at, ImVec2( bot.x, horizon ),
        rc_Fade( th.page, alpha ), rc_Fade( th.page, alpha ),
        rc_Fade( th.sunken, alpha ), rc_Fade( th.sunken, alpha ) );

    dl->AddRectFilledMultiColor( ImVec2( at.x, horizon ), bot,
        rc_Fade( th.sunken, alpha ), rc_Fade( th.sunken, alpha ),
        rc_Fade( th.page, alpha ), rc_Fade( th.page, alpha ) );

    // Two lights, drifting on periods that do not divide into each other, so
    // the pattern never repeats in a way anybody could catch.
    struct Glow { float x, y, r; ImU32 col; };
    Glow glows[ 2 ];

    glows[0].x = at.x + size.x * ( 0.30f + 0.16f * sinf( time * 0.045f ) );
    glows[0].y = at.y + size.y * ( 0.28f + 0.10f * cosf( time * 0.037f ) );
    glows[0].r = size.y * 0.55f;
    glows[0].col = th.primary.solid;

    glows[1].x = at.x + size.x * ( 0.76f + 0.13f * cosf( time * 0.031f ) );
    glows[1].y = at.y + size.y * ( 0.66f + 0.09f * sinf( time * 0.026f ) );
    glows[1].r = size.y * 0.45f;
    glows[1].col = th.second.solid;

    // Built from rings rather than an image: a dozen circles fading outwards
    // is indistinguishable from a soft light and costs nothing to carry.
    for ( int g = 0; g < 2; ++g )
    {
        int const rings = 14;
        for ( int i = rings; i > 0; --i )
        {
            float t = (float)i / (float)rings;
            float fade = ( 1.0f - t ) * ( 1.0f - t ) * 0.055f * alpha;
            dl->AddCircleFilled( ImVec2( glows[g].x, glows[g].y ),
                                 glows[g].r * t,
                                 rc_Fade( glows[g].col, fade ), 48 );
        }
    }

    // The floor. Lines running away to the horizon and rungs across them,
    // spaced so they crowd together as they recede - which is the whole of
    // what makes a flat screen read as distance.
    ImU32 grid = rc_Fade( th.second.solid, 0.085f * alpha );
    float middle = at.x + size.x * 0.5f;

    for ( int i = -14; i <= 14; ++i )
    {
        float spread = i * size.x * 0.075f;
        dl->AddLine( ImVec2( middle + spread * 0.16f, horizon ),
                     ImVec2( middle + spread * 2.4f, bot.y ), grid, 1.0f );
    }

    for ( int i = 1; i <= 12; ++i )
    {
        float t = (float)i / 12.0f;

        // Squared, so the gaps open up towards the viewer.
        float y = horizon + ( bot.y - horizon ) * t * t;
        dl->AddLine( ImVec2( at.x, y ), ImVec2( bot.x, y ),
                     rc_Fade( th.second.solid, 0.070f * t * alpha ), 1.0f );
    }

    // A line where the two meet, which is what sells the horizon.
    dl->AddLine( ImVec2( at.x, horizon ), ImVec2( bot.x, horizon ),
                 rc_Fade( th.second.solid, 0.16f * alpha ), 1.0f );

    // Corners taken down so the eye stays in the middle. Four bands rather
    // than a proper radial falloff, which nobody has ever noticed.
    float band = size.y * 0.22f;
    ImU32 dark = rc_Fade( IM_COL32( 0, 0, 0, 255 ), 0.55f * alpha );
    ImU32 gone = IM_COL32( 0, 0, 0, 0 );

    dl->AddRectFilledMultiColor( at, ImVec2( bot.x, at.y + band ), dark, dark, gone, gone );
    dl->AddRectFilledMultiColor( ImVec2( at.x, bot.y - band ), bot, gone, gone, dark, dark );
    dl->AddRectFilledMultiColor( at, ImVec2( at.x + band, bot.y ), dark, gone, gone, dark );
    dl->AddRectFilledMultiColor( ImVec2( bot.x - band, at.y ), bot, gone, dark, dark, gone );
}

bool rc_DrawCursor( bool pressed )
{
    ImGuiIO & io = ImGui::GetIO();
    rcTheme const & th = rc_Theme();

    ImVec2 at = io.MousePos;
    if ( at.x < -9000.0f || at.y < -9000.0f )
        return false;

    // A drawn pointer can only move when a frame is drawn. Fetching a server
    // list stops those for a moment, and in that moment the hand keeps moving
    // while the ring stays where it was, which looks broken. So when frames
    // stop arriving on time, ours stands down and the system's takes over
    // until things are running smoothly again.
    static float rough = 0.0f;
    if ( io.DeltaTime > 0.12f )
        rough = 0.45f;
    else if ( rough > 0.0f )
        rough -= io.DeltaTime;

    if ( rough > 0.0f )
        return false;

    ImDrawList * dl = ImGui::GetForegroundDrawList();

    // It tightens when a button goes down and lets go afterwards, which is
    // the whole of why a drawn pointer feels better than the system one: the
    // system's never answers.
    float press = rc_Ease( ImGui::GetID( "##rcCursorPress" ), pressed ? 1.0f : 0.0f, 0.09f );

    // A few frames of where it has been, thinning out. Enough to leave a
    // trace of the movement without turning into a comet.
    static ImVec2 was[ 5 ];
    static bool   started = false;
    if ( !started )
    {
        started = true;
        for ( int i = 0; i < 5; ++i )
            was[i] = at;
    }

    for ( int i = 4; i > 0; --i )
        was[i] = was[i-1];
    was[0] = at;

    for ( int i = 4; i > 0; --i )
    {
        float t = 1.0f - (float)i / 5.0f;
        dl->AddCircleFilled( was[i], 1.6f * t, rc_Fade( th.primary.solid, 0.13f * t ), 10 );
    }

    float ring = 9.0f - 2.2f * press;

    // Two rings rather than one: a dark one underneath means the pointer
    // stays readable over a pale panel as well as over the floor.
    dl->AddCircle( at, ring + 1.0f, IM_COL32( 0, 0, 0, 130 ), 24, 2.6f );
    dl->AddCircle( at, ring, rc_Fade( th.primary.solid, 0.92f ), 24, 1.5f );
    dl->AddCircleFilled( at, 1.9f + 0.8f * press, rc_Fade( th.text, 0.95f ), 10 );

    // A quarter turn of brighter arc that answers the press, so a click has
    // something of its own rather than only shrinking the ring.
    if ( press > 0.01f )
    {
        dl->PathArcTo( at, ring + 4.0f, 0.0f, 6.2831853f * press, 32 );
        dl->PathStroke( rc_Fade( th.primary.bright, 0.55f * press ), 0, 1.6f );
    }

    return true;
}

namespace
{

struct Note
{
    std::string text;
    float       born;
};

std::deque< Note > & Notes()
{
    static std::deque< Note > all;
    return all;
}

}

namespace
{

struct Flung
{
    ImVec2 at, way;
    float  born, size, lean;
};

std::deque< Flung > & Flock()
{
    static std::deque< Flung > all;
    return all;
}

}

void rc_HeartBurst()
{
    ImGuiIO & io = ImGui::GetIO();
    float now = tSysTimeFloat();

    for ( int i = 0; i < 26; ++i )
    {
        Flung one;

        // From along the bottom edge, thrown up and outwards, so they cross
        // the screen rather than appearing on top of it.
        one.at = ImVec2( io.DisplaySize.x * ( 0.1f + 0.8f * ( rand() % 1000 ) / 1000.0f ),
                         io.DisplaySize.y + 20.0f );
        one.way = ImVec2( (float)( ( rand() % 200 ) - 100 ) * 1.1f,
                          -( 420.0f + (float)( rand() % 300 ) ) );
        one.born = now;
        one.size = 9.0f + (float)( rand() % 12 );
        one.lean = (float)( rand() % 628 ) / 100.0f;

        Flock().push_back( one );
    }
}

void rc_Toast( char const * text, char const * sound )
{
    if ( !text || !*text )
        return;

    Note note;
    note.text = text;
    note.born = tSysTimeFloat();

    // Three at a time. Any more and they become a wall to read rather than a
    // thing to notice, which defeats the point of them.
    Notes().push_back( note );
    while ( Notes().size() > 3 )
        Notes().pop_front();

    if ( sound )
        se_PlayUi( sound );
}

void rc_DrawToasts()
{
    // The flock rides along with the notices: both are things laid over the
    // screen for a moment, and neither is worth a pass of its own.
    {
        std::deque< Flung > & flock = Flock();
        if ( !flock.empty() )
        {
            rcTheme const & th = rc_Theme();
            ImDrawList * dl = ImGui::GetForegroundDrawList();
            float now = tSysTimeFloat();
            float dt = ImGui::GetIO().DeltaTime;
            if ( dt > 0.05f ) dt = 0.05f;

            float const life = 2.6f;

            for ( size_t i = 0; i < flock.size(); ++i )
            {
                Flung & one = flock[i];
                float age = ( now - one.born ) / life;

                // Gravity, and a little turn as it goes, so none of them
                // travels in a way that looks calculated.
                one.way.y += 620.0f * dt;
                one.at.x += one.way.x * dt;
                one.at.y += one.way.y * dt;
                one.lean += dt * 1.4f;

                float fade = age > 0.6f ? ( 1.0f - age ) / 0.4f : 1.0f;
                if ( fade <= 0.0f )
                    continue;

                rc_DrawHeart( dl, one.at, one.size, one.lean,
                              rc_Fade( i % 3 == 0 ? th.primary.bright : th.primary.solid, fade * 0.9f ) );
            }

            while ( !flock.empty() && now - flock.front().born > life )
                flock.pop_front();
        }
    }

    std::deque< Note > & notes = Notes();
    if ( notes.empty() )
        return;

    rcTheme const & th = rc_Theme();
    ImDrawList * dl = ImGui::GetForegroundDrawList();
    ImGuiIO & io = ImGui::GetIO();

    float const life = 4.5f;
    float now = tSysTimeFloat();

    while ( !notes.empty() && now - notes.front().born > life )
        notes.pop_front();

    float y = 92.0f;

    for ( size_t i = 0; i < notes.size(); ++i )
    {
        Note const & note = notes[i];
        float age = ( now - note.born ) / life;

        // In quickly, steady for most of it, out slowly. Anything that leaves
        // as fast as it arrives reads as a glitch.
        float shown = 1.0f;
        if ( age < 0.10f )      shown = age / 0.10f;
        else if ( age > 0.75f ) shown = ( 1.0f - age ) / 0.25f;

        if ( shown <= 0.0f )
            continue;

        float w = ImGui::CalcTextSize( note.text.c_str() ).x + 44.0f;
        float h = ImGui::GetTextLineHeight() + 22.0f;

        // Slides in from the right edge it lives against.
        float slide = ( 1.0f - shown ) * 24.0f;
        ImVec2 at( io.DisplaySize.x - w - 28.0f + slide, y );
        ImVec2 to( at.x + w, at.y + h );

        dl->AddRectFilled( at, to, rc_Fade( th.raised, shown ), th.radiusMedium );
        dl->AddRect( at, to, rc_Fade( th.lineStrong, shown ), th.radiusMedium, 0, 1.0f );

        // The accent as a mark at the edge, the same as everywhere else that
        // something is worth pointing at.
        dl->AddRectFilled( ImVec2( at.x, at.y + 8.0f ), ImVec2( at.x + 3.0f, to.y - 8.0f ),
                           rc_Fade( th.primary.solid, shown ), 2.0f );

        dl->AddText( ImVec2( at.x + 18.0f, at.y + 11.0f ),
                     rc_Fade( th.text, shown ), note.text.c_str() );

        y += h + 10.0f;
    }
}
