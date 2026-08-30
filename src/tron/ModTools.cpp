#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef DEDICATED

#include "ModTools.h"
#include "ModTheme.h"
#include "eSound.h"

#include "imgui.h"

#include "tConfiguration.h"
#include "tString.h"
#include "eCamera.h"
#include "ePlayer.h"
#include "rScreen.h"
#include "rSysdep.h"
#include "rGL.h"

// The preview sets up its own projection and feeds vertices straight to the
// pipeline, which is exactly what this header forbids by default. It is the
// one place in the client that has to: everywhere else is drawing into the
// game's frame, and this is standing up a frame of its own.
#define DONTDOIT
#include "rRender.h"
#include "rTexture.h"
#include "eFloor.h"
#include "eWorldPaint.h"
#include "gCycle.h"
#include <math.h>

#include <sstream>
#include <string.h>
#include <stdio.h>
#include <string>
#include <vector>

namespace
{

//! Case insensitive "does this contain that".
bool Holds( std::string const & inside, std::string const & looking )
{
    if ( looking.empty() )
        return true;

    if ( looking.size() > inside.size() )
        return false;

    for ( size_t at = 0; at + looking.size() <= inside.size(); ++at )
    {
        size_t i = 0;
        while ( i < looking.size() &&
                tolower( (unsigned char)inside[ at + i ] ) == tolower( (unsigned char)looking[i] ) )
            ++i;

        if ( i == looking.size() )
            return true;
    }

    return false;
}

struct Command
{
    std::string name;
    std::string help;
    std::string value;
};

//! Taken once, when first asked for. The registry does not change while the
//! client is running, and walking a few thousand entries every frame to answer
//! the same question would be silly.
std::vector< Command > const & AllCommands()
{
    static std::vector< Command > all;
    static bool taken = false;

    if ( taken )
        return all;

    taken = true;

    tConfItemBase::tConfItemMap const & map = tConfItemBase::GetConfItemMap();

    for ( tConfItemBase::tConfItemMap::const_iterator i = map.begin(); i != map.end(); ++i )
    {
        tConfItemBase * item = i->second;
        if ( !item )
            continue;

        Command one;
        one.name = (char const *)item->GetTitle();

        // The help is written for the console, colour codes and all, and this
        // is not the console.
        tColoredString plain;
        plain << tString( item->GetHelp() );
        plain.RemoveHex();
        one.help = (char const *)plain;

        std::ostringstream value;
        item->WriteVal( value );
        one.value = value.str();

        all.push_back( one );
    }

    return all;
}

}

void rc_DrawCommandFinder( float width )
{
    rcTheme const & th = rc_Theme();

    static char looking[ 96 ] = "";

    std::vector< Command > const & all = AllCommands();

    rc_Tracked( "SERVER COMMAND FINDER", 1.6f, th.textMute );
    ImGui::Spacing();

    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.textDim ) );
    ImGui::TextWrapped( "Every setting this build knows, with what it is set to right now. "
                        "Searched across names and descriptions." );
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::SetNextItemWidth( width - 40.0f );
    ImGui::InputTextWithHint( "##rcCommandSearch", "cycle_speed, rubber, zone...", looking, sizeof( looking ) );

    // Counted before the table is drawn, so the line below it can say how much
    // of the list is being shown rather than how much exists.
    std::vector< Command const * > found;
    for ( size_t i = 0; i < all.size(); ++i )
        if ( Holds( all[i].name, looking ) || Holds( all[i].help, looking ) )
            found.push_back( &all[i] );

    ImGui::Spacing();
    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.textMute ) );
    ImGui::Text( "%d of %d", (int)found.size(), (int)all.size() );
    ImGui::PopStyleColor();
    ImGui::Spacing();

    if ( ImGui::BeginChild( "##rcCommandList", ImVec2( width - 20.0f, 420.0f ), true ) )
    {
        // Only the rows on screen are built. The list runs to a few thousand
        // and building all of them to show twenty is how a search box starts
        // to feel slow.
        ImGuiListClipper clipper;
        clipper.Begin( (int)found.size() );

        while ( clipper.Step() )
        {
            for ( int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row )
            {
                Command const & one = *found[ row ];

                ImGui::PushID( row );

                ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.text ) );
                ImGui::TextUnformatted( one.name.c_str() );
                ImGui::PopStyleColor();

                if ( !one.value.empty() )
                {
                    ImGui::SameLine();
                    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.second.solid ) );
                    ImGui::Text( "= %s", one.value.c_str() );
                    ImGui::PopStyleColor();
                }

                ImGui::SameLine( width - 110.0f );
                if ( ImGui::SmallButton( "copy" ) )
                {
                    std::string line = one.name + " " + one.value;
                    ImGui::SetClipboardText( line.c_str() );
                    se_PlayUi( "confirm" );
                }

                if ( !one.help.empty() )
                {
                    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.textDim ) );
                    ImGui::TextWrapped( "%s", one.help.c_str() );
                    ImGui::PopStyleColor();
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PopID();
            }
        }
    }

    ImGui::EndChild();
}

// ---------------------------------------------------------------- gradients

namespace
{

struct Stop
{
    float r, g, b;
};

std::vector< Stop > & Stops()
{
    static std::vector< Stop > all;

    if ( all.empty() )
    {
        Stop one = { 1.00f, 0.24f, 0.52f };
        Stop two = { 0.25f, 0.85f, 1.00f };
        all.push_back( one );
        all.push_back( two );
    }

    return all;
}

//! The colour at a point along the run, blending between whichever two stops
//! that point falls between.
Stop Blend( float along )
{
    std::vector< Stop > & stops = Stops();

    if ( stops.size() == 1 )
        return stops[0];

    if ( along <= 0.0f ) return stops.front();
    if ( along >= 1.0f ) return stops.back();

    float span = 1.0f / (float)( stops.size() - 1 );
    int first = (int)( along / span );
    if ( first >= (int)stops.size() - 1 )
        first = (int)stops.size() - 2;

    float t = ( along - first * span ) / span;

    Stop from = stops[ first ];
    Stop to = stops[ first + 1 ];

    Stop mixed;
    mixed.r = from.r + ( to.r - from.r ) * t;
    mixed.g = from.g + ( to.g - from.g ) * t;
    mixed.b = from.b + ( to.b - from.b ) * t;
    return mixed;
}

}

void rc_DrawGradientMaker( float width )
{
    rcTheme const & th = rc_Theme();

    static char written[ 64 ] = "ilonium";
    static bool hardEdges = false;

    std::vector< Stop > & stops = Stops();

    rc_Tracked( "GRADIENT NAME", 1.6f, th.textMute );
    ImGui::Spacing();

    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.textDim ) );
    ImGui::TextWrapped( "Colours run across the letters. Copy the result and paste it "
                        "wherever a name goes." );
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::SetNextItemWidth( width - 40.0f );
    ImGui::InputTextWithHint( "##rcGradientText", "your name", written, sizeof( written ) );

    ImGui::Spacing();

    // The colours, in the order they are laid down.
    int drop = -1;
    for ( size_t i = 0; i < stops.size(); ++i )
    {
        ImGui::PushID( (int)i );

        ImVec4 shown( stops[i].r, stops[i].g, stops[i].b, 1.0f );
        if ( ImGui::ColorEdit3( "##stop", (float *)&shown, ImGuiColorEditFlags_NoInputs ) )
        {
            stops[i].r = shown.x;
            stops[i].g = shown.y;
            stops[i].b = shown.z;
        }

        // Never below one: a gradient of nothing is not a thing, and removing
        // the last colour would leave the name with no colour to be written in.
        if ( stops.size() > 1 )
        {
            ImGui::SameLine();
            if ( ImGui::SmallButton( "x" ) )
                drop = (int)i;
        }

        if ( i + 1 < stops.size() )
            ImGui::SameLine();

        ImGui::PopID();
    }

    if ( drop >= 0 )
        stops.erase( stops.begin() + drop );

    ImGui::SameLine();
    if ( ImGui::SmallButton( "+" ) && stops.size() < 8 )
    {
        Stop fresh = Blend( 0.5f );
        stops.push_back( fresh );
    }

    ImGui::Spacing();
    ImGui::Checkbox( "Hard edges", &hardEdges );
    ImGui::SameLine();
    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.textMute ) );
    ImGui::TextUnformatted( hardEdges ? "each colour takes a share of the letters"
                                      : "the colours blend into each other" );
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();

    // What it will look like, drawn letter by letter in the colours the game
    // will use - not an approximation of them.
    std::string coded;
    int letters = (int)strlen( written );

    ImVec2 at = ImGui::GetCursorScreenPos();
    ImDrawList * dl = ImGui::GetWindowDrawList();
    float x = at.x;

    for ( int i = 0; i < letters; ++i )
    {
        float along = letters > 1 ? (float)i / (float)( letters - 1 ) : 0.0f;

        Stop paint;
        if ( hardEdges )
        {
            int which = (int)( along * ( stops.size() - 0.001f ) );
            if ( which < 0 ) which = 0;
            if ( which >= (int)stops.size() ) which = (int)stops.size() - 1;
            paint = stops[ which ];
        }
        else
        {
            paint = Blend( along );
        }

        char one[ 2 ] = { written[i], 0 };

        dl->AddText( ImVec2( x, at.y ), IM_COL32( (int)( paint.r * 255 ), (int)( paint.g * 255 ),
                                                  (int)( paint.b * 255 ), 255 ), one );
        x += ImGui::CalcTextSize( one ).x;

        // The game's own way of saying a colour: a marker followed by the
        // three parts in hexadecimal, then the letter it applies to.
        char code[ 16 ];
        snprintf( code, sizeof( code ), "0x%02x%02x%02x",
                  (int)( paint.r * 255 ), (int)( paint.g * 255 ), (int)( paint.b * 255 ) );

        coded += code;
        coded += one;
    }

    ImGui::Dummy( ImVec2( x - at.x, ImGui::GetTextLineHeight() ) );

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.textMute ) );
    ImGui::TextWrapped( "%s", coded.c_str() );
    ImGui::PopStyleColor();

    ImGui::Spacing();

    if ( ImGui::Button( "Copy name", ImVec2( 140.0f, 30.0f ) ) )
    {
        ImGui::SetClipboardText( coded.c_str() );
        se_PlayUi( "confirm" );
    }

    ImGui::SameLine();

    if ( ImGui::Button( "Copy as PLAYER_1", ImVec2( 190.0f, 30.0f ) ) )
    {
        std::string line = "PLAYER_1 " + coded;
        ImGui::SetClipboardText( line.c_str() );
        se_PlayUi( "confirm" );
    }
}

// ------------------------------------------------------------------ camera

// A view from the seat, rendered by the game rather than drawn to look like
// it: the cycle is the cycle, with its models and its textures, standing on
// the floor the game is set to use, inside the rim it is set to wear.
//
// The first version of this was a diagram, and a diagram is worthless here.
// The whole reason to preview a camera is to decide whether the picture it
// gives is the one you want to play in, and an invented picture can only tell
// you about itself.

//! What the arena itself is told about its walls. The preview asks the same
//! questions rather than guessing: a wall drawn at a height nobody uses looks
//! like a mistake, because it is one.
extern rFileTexture gWallRim_text;
extern bool g_EnhancedGraphicsMode;
extern REAL sg_modWallHeightMultiplier;
extern bool sg_modTrailGradientEnabled;
extern REAL sg_modTrailAlphaTop;
extern REAL sg_modTrailAlphaBottom;

namespace
{

void rc_ArenaRimTexture(){ gWallRim_text.Select(); }

bool rc_ArenaEnhanced(){ return g_EnhancedGraphicsMode; }
REAL rc_ArenaWallHeight(){ return sg_modWallHeightMultiplier; }
bool rc_ArenaTrailFade(){ return sg_modTrailGradientEnabled; }
REAL rc_ArenaTrailTop(){ return sg_modTrailAlphaTop; }
REAL rc_ArenaTrailFoot(){ return sg_modTrailAlphaBottom; }

//! What the preview is looking at this frame.
//!
//! The drawing happens inside an ImGui callback, well after the panel code has
//! returned, so the numbers have to be left somewhere for it. One panel, one
//! set of numbers.
struct rcStage
{
    ImVec2 at, size;
    rcCameraSet cam;
    int   fov;
    float scale;
    REAL  colour[ 3 ];
    REAL  trail[ 3 ];
};

rcStage s_stage;

void rc_StageGL( ImDrawList const *, ImDrawCmd const * )
{
    float s = s_stage.scale;

    int x = (int)( s_stage.at.x * s );
    int w = (int)( s_stage.size.x * s );
    int h = (int)( s_stage.size.y * s );
    int y = (int)( ( ImGui::GetIO().DisplaySize.y - s_stage.at.y - s_stage.size.y ) * s );

    if ( w < 4 || h < 4 )
        return;

    RenderEnd();

    // The vertex, texture and colour array pointers belong to the interface:
    // the backend sets them once for the whole draw list and never again, so
    // anything in here that touches them - the cycle model does - leaves every
    // later widget reading somebody else's vertices. They come back with the
    // client attributes, which the ordinary attribute stack does not carry.
    glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    glMatrixMode( GL_TEXTURE );
    glPushMatrix();
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();

    glViewport( x, y, w, h );
    glEnable( GL_SCISSOR_TEST );
    glScissor( x, y, w, h );

    glClearColor( 0, 0, 0, 1 );
    glClearDepth( 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glEnable( GL_DEPTH_TEST );
    glDepthMask( GL_TRUE );
    glDepthFunc( GL_LESS );
    glDisable( GL_CULL_FACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // The same frustum the viewport builds, so the framing here is the framing
    // the game will give.
    float aspect = (float)w / (float)h;
    float ensure = aspect / 1.5f > 1.0f ? aspect / 1.5f : 1.0f;
    float xmul = ensure * tanf( 0.0087266463f * (float)s_stage.fov );
    float ymul = xmul / aspect;
    // Depth precision is spent almost entirely on the first few units, so a
    // near plane this close and a far plane at a hundred thousand leaves the
    // machine's own parts fighting each other. The arena is two hundred units
    // across; there is nothing out at a hundred thousand to keep.
    float znear = 1.0f;

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -znear * xmul, znear * xmul, -znear * ymul, znear * ymul, znear, 4000.0f );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt( -s_stage.cam.back, 0, s_stage.cam.rise,
               -s_stage.cam.back + 1, 0, s_stage.cam.rise + s_stage.cam.pitch,
               0, 0, 1 );

    // ---- the floor, drawn the way the game is currently set to draw it
    if ( rc_NeonFloor() || !eFloor::Floor )
    {
        REAL const reach = 1400;

        glDisable( GL_TEXTURE_2D );

        rcPaint const & paint = rc_FloorPaint();
        glColor4f( paint.r, paint.g, paint.b, 0.95f );

        glBegin( GL_QUADS );
        glVertex3f( -reach, -reach, 0 );
        glVertex3f(  reach, -reach, 0 );
        glVertex3f(  reach,  reach, 0 );
        glVertex3f( -reach,  reach, 0 );
        glEnd();

        rc_PaintNeonGrid();

        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glLineWidth( 1.0f );
    }
    else if ( sr_floorDetail == rFLOOR_GRID )
    {
        REAL side = se_GridSize();
        if ( side <= 0 ) side = 4;

        glDisable( GL_TEXTURE_2D );
        glBegin( GL_LINES );
        for ( int i = -40; i <= 40; ++i )
        {
            REAL intens = 1 - ( i * i ) / 1600.0f;
            if ( intens < 0 ) intens = 0;

            se_glFloorColor( intens, intens );
            glVertex3f( i * side, -41 * side, 0 );
            glVertex3f( i * side, 41 * side, 0 );
            glVertex3f( -41 * side, i * side, 0 );
            glVertex3f( 41 * side, i * side, 0 );
        }
        glEnd();
    }
    else if ( sr_floorDetail >= rFLOOR_TEXTURE )
    {
        REAL gs = se_GridSize();
        if ( gs <= 0 ) gs = 1;

        glMatrixMode( GL_TEXTURE );
        glLoadIdentity();
        glScalef( 1 / gs, 1 / gs, 1 / gs );
        glMatrixMode( GL_MODELVIEW );

        glEnable( GL_TEXTURE_2D );
        se_glFloorTexture();
        se_glFloorColor( 1 );

        REAL const reach = 1400;

        glBegin( GL_QUADS );
        glTexCoord2f( -reach, -reach ); glVertex3f( -reach, -reach, 0 );
        glTexCoord2f(  reach, -reach ); glVertex3f(  reach, -reach, 0 );
        glTexCoord2f(  reach,  reach ); glVertex3f(  reach,  reach, 0 );
        glTexCoord2f( -reach,  reach ); glVertex3f( -reach,  reach, 0 );
        glEnd();

        glMatrixMode( GL_TEXTURE );
        glLoadIdentity();
        glMatrixMode( GL_MODELVIEW );
    }

    // ---- the arena, so there is somewhere for all this to be happening
    {
        REAL const edge = 200, tall = 8;

        rc_ArenaRimTexture();
        glEnable( GL_TEXTURE_2D );
        glColor4f( 1, 1, 1, 1 );

        REAL corner[ 4 ][ 2 ] = { { -edge, -edge }, { edge, -edge }, { edge, edge }, { -edge, edge } };

        glBegin( GL_QUADS );
        for ( int i = 0; i < 4; ++i )
        {
            REAL * a = corner[ i ];
            REAL * b = corner[ ( i + 1 ) % 4 ];
            REAL run = sqrtf( ( b[0] - a[0] ) * ( b[0] - a[0] ) + ( b[1] - a[1] ) * ( b[1] - a[1] ) ) / tall;

            glTexCoord2f( 0, 1 );   glVertex3f( a[0], a[1], 0 );
            glTexCoord2f( run, 1 ); glVertex3f( b[0], b[1], 0 );
            glTexCoord2f( run, 0 ); glVertex3f( b[0], b[1], tall );
            glTexCoord2f( 0, 0 );   glVertex3f( a[0], a[1], tall );
        }
        glEnd();
    }

    // ---- the wall this cycle is laying, straight out behind it
    //
    // A camera sitting directly behind sees it edge on, so what shows is its
    // lit top edge and nothing else. That is not a shortcoming of the preview:
    // it is what the game gives you, and how little of your own wall you can
    // see is one of the things these numbers decide.
    {
        // the height the arena would give it, not one picked to look right
        REAL tall = rc_ArenaEnhanced() ? rc_ArenaWallHeight() : 1.0f;
        REAL const length = 150;

        // A wall leaves the back of the cycle, not its middle. The model is
        // drawn about its own centre, so starting the wall at zero pushed it
        // through the hull and out the front, which is the one thing that never
        // happens in a real round.
        REAL const behind = -0.75f;

        bool fade = rc_ArenaEnhanced() && rc_ArenaTrailFade();
        REAL top = fade ? rc_ArenaTrailTop() : 1.0f;
        REAL foot = fade ? rc_ArenaTrailFoot() : 1.0f;

        glDisable( GL_TEXTURE_2D );

        REAL r = s_stage.trail[0], g = s_stage.trail[1], b = s_stage.trail[2];

        glBegin( GL_QUADS );
        glColor4f( r, g, b, foot );
        glVertex3f( behind, 0, 0 );
        glVertex3f( -length, 0, 0 );
        glColor4f( r, g, b, top );
        glVertex3f( -length, 0, tall );
        glVertex3f( behind, 0, tall );
        glEnd();

        glLineWidth( 1.4f );
        glBegin( GL_LINES );
        glColor4f( r, g, b, top );
        glVertex3f( behind, 0, tall );
        glVertex3f( -length, 0, tall );
        glEnd();
        glLineWidth( 1.0f );
    }

    // ---- and the cycle itself
    {
        GLfloat white[ 4 ] = { 1, 1, 1, 1 };
        GLfloat lposa[ 4 ] = { 320, 240, 200, 0 };
        GLfloat lposb[ 4 ] = { -240, -100, 200, 0 };
        GLfloat lighta[ 4 ] = { 1, .7f, .7f, 1 };
        GLfloat lightb[ 4 ] = { .7f, .7f, 1, 1 };

        glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, white );
        glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, white );

        glLightfv( GL_LIGHT0, GL_DIFFUSE, lighta );
        glLightfv( GL_LIGHT0, GL_SPECULAR, lighta );
        glLightfv( GL_LIGHT0, GL_POSITION, lposa );
        glLightfv( GL_LIGHT1, GL_DIFFUSE, lightb );
        glLightfv( GL_LIGHT1, GL_SPECULAR, lightb );
        glLightfv( GL_LIGHT1, GL_POSITION, lposb );

        glEnable( GL_LIGHT0 );
        glEnable( GL_LIGHT1 );
        glEnable( GL_LIGHTING );

        rc_RenderCycleModel( s_stage.colour[0], s_stage.colour[1], s_stage.colour[2] );

        glDisable( GL_LIGHTING );
    }

    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_TEXTURE );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );

    glPopAttrib();
    glPopClientAttrib();
}

}

void rc_DrawCameraTool( float width )
{
    rcTheme const & th = rc_Theme();

    ePlayer * seat = ePlayer::PlayerConfig( 0 );

    // A bench, not a control panel. Nothing here touches the camera you are
    // playing with: you build a set of numbers, look at them, and take them
    // away as config lines. The one you fly with only changes when you say so.
    static bool started = false;
    static rcCameraSet edit;
    static int editFov = 90;
    static float ride = 30.0f;

    if ( !started )
    {
        started = true;
        rc_CustomCameraGet( edit );
        editFov = seat ? seat->startFOV : 90;
    }

    rc_Tracked( "CAMERA", 1.6f, th.textMute );
    ImGui::Spacing();

    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.textDim ) );
    ImGui::TextWrapped( "The game's own cycle and floor, framed by the numbers below. "
                        "Nothing here changes the camera you play with - copy the lines "
                        "out when you like what you see." );
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // What the camera is actually at right now: the two speed numbers move it
    // back and up the faster you go, so at a standstill they do nothing and
    // the picture would never match the one you ride in.
    rcCameraSet shot = edit;
    shot.back += ride * edit.backFromSpeed;
    shot.rise += ride * edit.riseFromSpeed;

    float room = width - 30.0f;
    float roomH = ImGui::GetContentRegionAvail().y - 270.0f;
    if ( roomH < 280.0f ) roomH = 280.0f;
    if ( roomH > 520.0f ) roomH = 520.0f;

    float shape = 16.0f / 9.0f;
    if ( sr_screenHeight > 0 )
        shape = ( (float)sr_screenWidth * currentScreensetting.aspect ) / (float)sr_screenHeight;
    if ( shape < 0.5f ) shape = 0.5f;

    float viewW = room, viewH = room / shape;
    if ( viewH > roomH )
    {
        viewH = roomH;
        viewW = roomH * shape;
    }

    ImVec2 outer = ImGui::GetCursorScreenPos();
    ImVec2 at( outer.x + ( room - viewW ) * 0.5f, outer.y + ( roomH - viewH ) * 0.5f );
    ImVec2 bot( at.x + viewW, at.y + viewH );

    s_stage.at = at;
    s_stage.size = ImVec2( viewW, viewH );
    s_stage.cam = shot;
    s_stage.fov = editFov;

    ImGuiIO & io = ImGui::GetIO();
    s_stage.scale = io.DisplayFramebufferScale.y > 0.0f ? io.DisplayFramebufferScale.y : 1.0f;

    s_stage.colour[0] = s_stage.colour[1] = s_stage.colour[2] = 1.0f;
    s_stage.trail[0] = s_stage.trail[1] = s_stage.trail[2] = 1.0f;

    if ( seat && seat->netPlayer )
    {
        seat->netPlayer->Color( s_stage.colour[0], s_stage.colour[1], s_stage.colour[2] );
        seat->netPlayer->TrailColor( s_stage.trail[0], s_stage.trail[1], s_stage.trail[2] );
    }
    else if ( seat )
    {
        for ( int i = 0; i < 3; ++i )
            s_stage.colour[i] = s_stage.trail[i] = seat->rgb[i] / 15.0f;
    }

    ImDrawList * dl = ImGui::GetWindowDrawList();
    dl->AddCallback( rc_StageGL, NULL );
    dl->AddCallback( ImDrawCallback_ResetRenderState, NULL );

    dl->AddRect( at, bot, th.line, th.radiusMedium, 0, 1.0f );

    {
        float wide = ( shape / 1.5f > 1.0f ) ? shape / 1.5f : 1.0f;

        char said[ 128 ];
        snprintf( said, sizeof( said ), "%d\xc2\xb0 across    %.0f\xc2\xb0 down    %.1f behind    %.1f up    at speed %.0f",
                  (int)( 2.0f * atanf( wide * tanf( 0.0087266463f * (float)editFov ) ) * 57.29578f ),
                  -atanf( shot.pitch ) * 57.29578f, shot.back, shot.rise, ride );

        dl->AddText( ImVec2( at.x + 12.0f, bot.y - 24.0f ), rc_Fade( th.text, 0.75f ), said );
    }

    ImGui::Dummy( ImVec2( room, roomH ) );
    ImGui::Spacing();

    float column = ( width - 60.0f ) * 0.5f;

    ImGui::BeginGroup();
    ImGui::SetNextItemWidth( column - 120.0f );
    ImGui::SliderInt( "Field of view", &editFov, 40, 160 );

    ImGui::SetNextItemWidth( column - 120.0f );
    ImGui::SliderFloat( "Pitch", &edit.pitch, -1.5f, 0.5f, "%.2f" );

    ImGui::SetNextItemWidth( column - 120.0f );
    ImGui::SliderFloat( "Rise", &edit.rise, 0.0f, 120.0f, "%.1f" );

    ImGui::SetNextItemWidth( column - 120.0f );
    ImGui::SliderFloat( "Back", &edit.back, 0.0f, 120.0f, "%.1f" );
    ImGui::EndGroup();

    ImGui::SameLine( column + 30.0f );

    ImGui::BeginGroup();
    ImGui::SetNextItemWidth( column - 170.0f );
    ImGui::SliderFloat( "Additional rise speed", &edit.riseFromSpeed, 0.0f, 2.0f, "%.2f" );

    ImGui::SetNextItemWidth( column - 170.0f );
    ImGui::SliderFloat( "Additional back speed", &edit.backFromSpeed, 0.0f, 2.0f, "%.2f" );

    ImGui::SetNextItemWidth( column - 170.0f );
    ImGui::SliderFloat( "Turn speed", &edit.turnSpeed, 0.0f, 100.0f, "%.0f" );

    ImGui::SetNextItemWidth( column - 170.0f );
    ImGui::SliderFloat( "Additional turn speed", &edit.turnSpeed180, 0.0f, 20.0f, "%.1f" );
    ImGui::EndGroup();

    ImGui::Spacing();

    ImGui::SetNextItemWidth( column - 120.0f );
    ImGui::SliderFloat( "Cycle speed", &ride, 0.0f, 80.0f, "%.0f" );

    ImGui::Spacing();

    if ( ImGui::Button( "Copy config", ImVec2( 150.0f, 30.0f ) ) )
    {
        char lines[ 512 ];
        snprintf( lines, sizeof( lines ),
                  "CAMERA_CUSTOM_BACK %.2f\n"
                  "CAMERA_CUSTOM_RISE %.2f\n"
                  "CAMERA_CUSTOM_PITCH %.2f\n"
                  "CAMERA_CUSTOM_BACK_FROMSPEED %.2f\n"
                  "CAMERA_CUSTOM_RISE_FROMSPEED %.2f\n"
                  "CAMERA_CUSTOM_TURN_SPEED %.2f\n"
                  "CAMERA_CUSTOM_TURN_SPEED_180 %.2f\n"
                  "START_FOV_1 %d\n",
                  edit.back, edit.rise, edit.pitch,
                  edit.backFromSpeed, edit.riseFromSpeed,
                  edit.turnSpeed, edit.turnSpeed180, editFov );

        ImGui::SetClipboardText( lines );
        se_PlayUi( "confirm" );
    }

    ImGui::SameLine();

    if ( ImGui::Button( "Load mine", ImVec2( 150.0f, 30.0f ) ) )
    {
        rc_CustomCameraGet( edit );
        editFov = seat ? seat->startFOV : 90;
        se_PlayUi( "click" );
    }

    ImGui::SameLine();

    if ( ImGui::Button( "Apply to game", ImVec2( 150.0f, 30.0f ) ) )
    {
        rc_CustomCameraSet( edit );
        rc_CameraFOV( editFov );
        rc_Toast( "camera applied", "confirm" );
    }
}

#endif // DEDICATED
