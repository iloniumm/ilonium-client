#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef DEDICATED

#include "ModPalette.h"
#include "ModTheme.h"
#include "eSound.h"
#include "HudManager.h"

#include "imgui.h"

#include <algorithm>
#include <string>
#include <vector>

namespace
{

struct Place
{
    int         what;
    std::string label;
    std::string category;
};

std::vector< Place > & Places()
{
    static std::vector< Place > all;
    return all;
}

rcPaletteGo go = NULL;

bool  up = false;
float shown = 0.0f;          //!< how far open, eased, so it arrives rather than appears
char  typed[ 96 ] = "";
int   picked = 0;
bool  takeFocus = false;

struct Hit
{
    int   place;
    int   score;
    bool  operator < ( Hit const & other ) const { return score > other.score; }
};

//! Scores how well a query fits a phrase, or returns nothing when it does not.
//!
//! Letters have to appear in order but not together, so "srvbr" finds "Server
//! Browser". Landing on the start of a word counts for much more than landing
//! in the middle of one, which is what keeps the obvious answer at the top
//! instead of some longer entry that happens to contain the same letters.
bool Score( std::string const & phrase, char const * query, int & out )
{
    if ( !query || !*query )
    {
        out = 0;
        return true;
    }

    int total = 0;
    size_t at = 0;
    bool wordStart = true;
    int run = 0;

    for ( char const * q = query; *q; ++q )
    {
        char want = (char)tolower( (unsigned char)*q );
        if ( want == ' ' )
            continue;

        bool found = false;
        while ( at < phrase.size() )
        {
            char have = (char)tolower( (unsigned char)phrase[ at ] );
            bool starts = ( at == 0 ) ||
                          phrase[ at - 1 ] == ' ' ||
                          phrase[ at - 1 ] == '/';

            ++at;

            if ( have == want )
            {
                total += starts ? 12 : 3;
                total += run;              // letters found together are worth more
                run += 2;
                wordStart = starts;
                found = true;
                break;
            }

            run = 0;
        }

        if ( !found )
            return false;
    }

    (void)wordStart;

    // Shorter answers win ties: given "conf", the section is more likely
    // wanted than a longer entry that also contains it.
    total -= (int)phrase.size() / 6;

    out = total;
    return true;
}

std::vector< Hit > Matches()
{
    std::vector< Hit > hits;
    std::vector< Place > const & all = Places();

    for ( size_t i = 0; i < all.size(); ++i )
    {
        int best = -10000;
        int score = 0;

        if ( Score( all[i].label, typed, score ) && score > best )
            best = score;

        // The category counts for less than the name, so typing "settings"
        // lists them all without a section ever outranking its own title.
        if ( Score( all[i].category, typed, score ) && score - 6 > best )
            best = score - 6;

        if ( best > -10000 )
        {
            Hit hit;
            hit.place = (int)i;
            hit.score = best;
            hits.push_back( hit );
        }
    }

    std::stable_sort( hits.begin(), hits.end() );
    return hits;
}

}

void rc_PaletteAdd( int what, char const * label, char const * category )
{
    if ( !label )
        return;

    Place place;
    place.what     = what;
    place.label    = label;
    place.category = category ? category : "";
    Places().push_back( place );
}

void rc_PaletteHandler( rcPaletteGo handler )
{
    go = handler;
}

bool rc_PaletteVisible()
{
    return up;
}

void rc_PaletteClose()
{
    if ( !up )
        return;
    up = false;
    se_PlayUi( "back" );
}

void rc_PaletteToggle()
{
    if ( up )
    {
        rc_PaletteClose();
        return;
    }

    up = true;
    typed[ 0 ] = 0;
    picked = 0;
    takeFocus = true;
    se_PlayUi( "open" );
}

void rc_PaletteDraw()
{
    rcTheme const & th = rc_Theme();

    shown = rc_Ease( ImGui::GetID( "##rcPalette" ), up ? 1.0f : 0.0f, th.settle );
    if ( shown < 0.002f )
        return;

    ImGuiIO & io = ImGui::GetIO();

    // All of it inside one window of its own, drawn last so it sits above the
    // screen it covers. An earlier attempt put the card in the layer that goes
    // over everything, which also went over the field, so what was typed was
    // hidden behind its own background.
    ImGui::SetNextWindowPos( ImVec2( 0, 0 ) );
    ImGui::SetNextWindowSize( io.DisplaySize );

    ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0, 0, 0, 0 ) );
    ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0, 0, 0, 0 ) );
    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4( th.text ) );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );
    // The field is a line of text with a mark beside it, not a control in a
    // box: the card is already the box.
    ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 0.0f );

    ImGui::Begin( "##rcPaletteWindow", NULL,
                  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                  ImGuiWindowFlags_NoScrollWithMouse |
                  ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground );

    ImDrawList * dl = ImGui::GetWindowDrawList();

    // Everything behind goes back a step rather than away, so it still reads
    // as the same screen with something on top of it.
    dl->AddRectFilled( ImVec2( 0, 0 ), io.DisplaySize,
                       IM_COL32( 4, 2, 8, (int)( 170 * shown ) ) );

    float width = 660.0f;
    float rowH = 42.0f;

    std::vector< Hit > hits = Matches();
    int rows = (int)hits.size();
    if ( rows > 7 )
        rows = 7;

    float inputH = 62.0f;
    // Room for the "nothing found" line too, so the card does not shrink to a
    // bare field the moment a search misses.
    float height = inputH;
    if ( rows > 0 )
        height += rows * rowH + 12.0f;
    else if ( typed[0] )
        height += 46.0f;

    // Arrives from slightly small and slightly high. A panel that simply
    // appears at full size reads as a screenshot pasted over the screen.
    float ease = shown * shown * ( 3.0f - 2.0f * shown );
    ImVec2 size( width, height );
    ImVec2 at( ( io.DisplaySize.x - size.x ) * 0.5f,
               io.DisplaySize.y * 0.30f + ( 1.0f - ease ) * 18.0f );
    ImVec2 bot( at.x + size.x, at.y + size.y );

    dl->AddRectFilled( at, bot, rc_Fade( th.raised, ease ), th.radiusLarge );
    dl->AddRect( at, bot, rc_Fade( th.lineStrong, ease ), th.radiusLarge, 0, 1.0f );
    dl->AddLine( ImVec2( at.x + th.radiusLarge, at.y + 0.5f ),
                 ImVec2( bot.x - th.radiusLarge, at.y + 0.5f ),
                 rc_Fade( th.primary.solid, 0.35f * ease ), 1.0f );

    // A mark where the typing goes, in place of a box around the field. One
    // border inside another is one border too many.
    dl->AddRectFilled( ImVec2( at.x + 22.0f, at.y + inputH * 0.5f - 9.0f ),
                       ImVec2( at.x + 24.5f, at.y + inputH * 0.5f + 9.0f ),
                       rc_Fade( th.primary.solid, ease ), 2.0f );

    ImGui::SetCursorScreenPos( ImVec2( at.x + 38.0f, at.y + inputH * 0.5f - 13.0f ) );

    if ( takeFocus )
    {
        ImGui::SetKeyboardFocusHere();
        takeFocus = false;
    }

    ImGui::PushFont( g_FontHeader ? g_FontHeader : ImGui::GetFont() );
    ImGui::SetNextItemWidth( width - 76.0f );
    if ( ImGui::InputTextWithHint( "##rcPaletteQuery", "where to?", typed, sizeof( typed ) ) )
        picked = 0;
    ImGui::PopFont();

    bool chose = ImGui::IsKeyPressed( ImGuiKey_Enter, false ) ||
                 ImGui::IsKeyPressed( ImGuiKey_KeypadEnter, false );

    if ( ImGui::IsKeyPressed( ImGuiKey_DownArrow, true ) )
    {
        ++picked;
        se_PlayUi( "hover" );
    }
    if ( ImGui::IsKeyPressed( ImGuiKey_UpArrow, true ) )
    {
        --picked;
        se_PlayUi( "hover" );
    }
    if ( ImGui::IsKeyPressed( ImGuiKey_Escape, false ) )
        rc_PaletteClose();

    if ( !hits.empty() )
    {
        if ( picked < 0 ) picked = (int)hits.size() - 1;
        if ( picked >= (int)hits.size() ) picked = 0;
    }
    else
    {
        picked = 0;
    }

    float lineY = at.y + inputH - 1.0f;
    dl->AddLine( ImVec2( at.x + 20.0f, lineY ), ImVec2( bot.x - 20.0f, lineY ),
                 rc_Fade( th.line, ease ), 1.0f );

    for ( int i = 0; i < rows; ++i )
    {
        Place const & place = Places()[ hits[ i ].place ];

        float rowY = lineY + 8.0f + i * rowH;
        ImVec2 rowMin( at.x + 10.0f, rowY );
        ImVec2 rowMax( bot.x - 10.0f, rowY + rowH - 4.0f );

        bool over = io.MousePos.x >= rowMin.x && io.MousePos.x <= rowMax.x &&
                    io.MousePos.y >= rowMin.y && io.MousePos.y <= rowMax.y;

        if ( over && ( io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f ) && picked != i )
        {
            picked = i;
            se_PlayUi( "hover" );
        }

        bool here = ( i == picked );

        // Each row arrives a moment after the one above it. The delay is
        // small enough not to be waited for and large enough to be felt.
        float step = rc_Ease( ImGui::GetID( place.label.c_str() ) ^ 0x51EDu,
                              up ? 1.0f : 0.0f, th.settle + i * 0.035f );

        float slide = ( 1.0f - step ) * 10.0f;
        ImVec2 shift( rowMin.x + slide, rowMin.y );
        ImVec2 shiftMax( rowMax.x + slide, rowMax.y );

        if ( here )
        {
            dl->AddRectFilled( shift, shiftMax, rc_Fade( th.primary.wash, step ), th.radiusSmall );

            float half = ( rowH - 4.0f ) * 0.3f;
            float centre = ( shift.y + shiftMax.y ) * 0.5f;
            dl->AddRectFilled( ImVec2( shift.x, centre - half ),
                               ImVec2( shift.x + 2.5f, centre + half ),
                               rc_Fade( th.primary.solid, step ), 2.0f );
        }

        float textY = rowY + ( rowH - 4.0f - ImGui::GetTextLineHeight() ) * 0.5f;
        dl->AddText( ImVec2( shift.x + 18.0f, textY ),
                     rc_Fade( here ? th.text : th.textDim, step ),
                     place.label.c_str() );

        if ( !place.category.empty() )
        {
            float w = ImGui::CalcTextSize( place.category.c_str() ).x;
            dl->AddText( ImVec2( shiftMax.x - 16.0f - w, textY ),
                         rc_Fade( th.textMute, step * 0.9f ),
                         place.category.c_str() );
        }

        if ( over && io.MouseClicked[ 0 ] )
            chose = true;
    }

    // Nothing matched: say so where the rows would have been, rather than
    // leaving a card that looks broken.
    if ( rows == 0 && typed[0] )
    {
        char const * none = "nothing by that name";
        float w = ImGui::CalcTextSize( none ).x;
        dl->AddText( ImVec2( at.x + ( width - w ) * 0.5f, lineY + 16.0f ),
                     rc_Fade( th.textMute, ease ), none );
    }

    ImGui::End();
    ImGui::PopStyleVar( 3 );
    ImGui::PopStyleColor( 3 );

    if ( chose && up )
    {
        if ( !hits.empty() && go )
        {
            se_PlayUi( "confirm" );
            int what = Places()[ hits[ picked ].place ].what;
            up = false;
            go( what );
        }
        else
        {
            // Nothing matched - but a few words are worth typing anyway, and
            // somebody who stumbles on one should feel like they found
            // something rather than like they mistyped.
            std::string what = typed;
            for ( size_t i = 0; i < what.size(); ++i )
                what[i] = (char)tolower( (unsigned char)what[i] );

            if ( what == "ilonium" || what == "love" || what == "<3" )
            {
                rc_HeartBurst();
                rc_Toast( what == "<3" ? "<3" : "made with love", "confirm" );
                up = false;
            }
            else
            {
                se_PlayUi( "deny" );
            }
        }
    }
}

#endif // DEDICATED
