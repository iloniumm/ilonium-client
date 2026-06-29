/*

*************************************************************************

ArmageTron -- Just another Tron Lightcycle Game in 3D.
Copyright (C) 2000  Manuel Moos (manuel@moosnet.de)

**************************************************************************

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

***************************************************************************

*/

// I don't know if these includes are needed, I'm brute-forcing it :) - Dave
#include "gMenus.h"
#include "ePlayer.h"
#include "rScreen.h"
#include "nConfig.h"
#include "rConsole.h"
#include "tToDo.h"
#include "rGL.h"
#include "eTimer.h"
#include "eFloor.h"
#include "rRender.h"
#include "rModel.h"
#include "gGame.h"
#include "tRecorder.h"

#include "rRender.h"
#include <math.h>
#include "gCycle.h"
#include <time.h>

#include "gHud.h"

#include "eRectangle.h"
#include "eAdvWall.h"
#include "gWall.h"
#include "gWinZone.h"
#include "eGrid.h"
#include "eTeam.h"
#include <map>
#include <vector>
#include <algorithm>

static bool show_minimap_mod = true;
static void display_minimap_subby(ePlayerNetID* me)
{
    if (!show_minimap_mod || !me || !me->Object() || !me->Object()->Alive())
        return;
    
    // Reset matrices using rRender wrapper
    ProjMatrix();
    IdentityMatrix();
    ModelMatrix();
    IdentityMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_LINE_SMOOTH);
    
    const eRectangle &bounds = eWallRim::GetBounds();
    eCoord center;
    center.x = (bounds.GetLow().x + bounds.GetHigh().x) * 0.5f;
    center.y = (bounds.GetLow().y + bounds.GetHigh().y) * 0.5f;
    
    // Viewport dimensions
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    float aspect = viewport[2] / (float)viewport[3];
    
    // Base size scaled by mod setting (anchored to bottom-right)
    float frac_y = 0.20f * sg_modMinimapScale;
    
    // Calculate a 2-pixel margin in screen space [-1, 1]
    float margin_x = 2.0f / (float)viewport[2] * 2.0f;
    float margin_y = 2.0f / (float)viewport[3] * 2.0f;
    
    float arena_w = bounds.GetHigh().x - bounds.GetLow().x;
    float arena_h = bounds.GetHigh().y - bounds.GetLow().y;
    float game_radius_x = arena_w * 0.5f;
    float game_radius_y = arena_h * 0.5f;
    if (game_radius_x < 10.0f) game_radius_x = 10.0f;
    if (game_radius_y < 10.0f) game_radius_y = 10.0f;
    
    float max_game_radius = (game_radius_x > game_radius_y) ? game_radius_x : game_radius_y;
    
    // Inner zoom: zooms content inside minimap, always centered on arena center
    float innerZoom = sg_modMinimapZoom;
    if (innerZoom < 1.0f) innerZoom = 1.0f;
    
    // Base zoom (fits full arena into minimap)
    float base_zoom_y = frac_y / max_game_radius;
    float base_zoom_x = base_zoom_y / aspect;
    
    // Actual zoom with inner zoom applied
    float zoom_y = base_zoom_y * innerZoom;
    float zoom_x = base_zoom_x * innerZoom;
    
    // Minimap screen-space size (stays the same regardless of inner zoom)
    float map_screen_w = base_zoom_x * arena_w;
    float map_screen_h = base_zoom_y * arena_h;
    
    // Anchor: right-bottom corner stays fixed
    float right_edge = 1.0f - margin_x;
    float bottom_edge = -1.0f + margin_y;
    
    // Minimap center in screen coords
    float map_cx = right_edge - map_screen_w * 0.5f;
    float map_cy = bottom_edge + map_screen_h * 0.5f;
    
    // Offset: arena center always maps to minimap center (static!)
    float offset_x = map_cx;
    float offset_y = map_cy;
    
    // Transform world coords to screen coords (with optional rotation)
    float rotCos = 1.0f, rotSin = 0.0f;
    if (sg_modMinimapRotate && me->Object()) {
        gCycle *myCycle = dynamic_cast<gCycle*>(me->Object());
        if (myCycle) {
            eCoord dir = myCycle->Direction();
            // Rotate so player faces UP on minimap
            // Player dir is (dx,dy), we want to rotate by -atan2(dx,dy)+90
            float len = sqrt(dir.x*dir.x + dir.y*dir.y);
            if (len > 0.001f) {
                rotCos = dir.y / len;   // cos(-angle+90)
                rotSin = -dir.x / len;  // sin(-angle+90)
            }
        }
    }
    
    auto tx = [&](float wx, float wy) -> float {
        float dx = wx - center.x;
        float dy = wy - center.y;
        // Rotate in world space (uniform), then scale to screen
        float rx = dx * rotCos - dy * rotSin;
        return offset_x + rx * zoom_x;
    };
    auto ty = [&](float wx, float wy) -> float {
        float dx = wx - center.x;
        float dy = wy - center.y;
        // Rotate in world space (uniform), then scale to screen
        float ry = dx * rotSin + dy * rotCos;
        return offset_y + ry * zoom_y;
    };
    
    // Scissor: clip to minimap bounds (with 1px padding for edge walls)
    float clip_left = map_cx - map_screen_w * 0.5f;
    float clip_bot = map_cy - map_screen_h * 0.5f;
    float clip_right = map_cx + map_screen_w * 0.5f;
    float clip_top = map_cy + map_screen_h * 0.5f;
    
    int sci_x = (int)((clip_left + 1.0f) * 0.5f * viewport[2]) + viewport[0] - 1;
    int sci_y = (int)((clip_bot + 1.0f) * 0.5f * viewport[3]) + viewport[1] - 1;
    int sci_w = (int)((clip_right - clip_left) * 0.5f * viewport[2]) + 2;
    int sci_h = (int)((clip_top - clip_bot) * 0.5f * viewport[3]) + 2;
    
    glEnable(GL_SCISSOR_TEST);
    glScissor(sci_x, sci_y, sci_w, sci_h);
    
    // Black background perfectly matching arena bounds
    BeginQuads();
    Color(0.0f, 0.0f, 0.0f, 0.85f);
    Vertex(tx(bounds.GetLow().x, bounds.GetLow().y), ty(bounds.GetLow().x, bounds.GetLow().y));
    Vertex(tx(bounds.GetHigh().x, bounds.GetLow().y), ty(bounds.GetHigh().x, bounds.GetLow().y));
    Vertex(tx(bounds.GetHigh().x, bounds.GetHigh().y), ty(bounds.GetHigh().x, bounds.GetHigh().y));
    Vertex(tx(bounds.GetLow().x, bounds.GetHigh().y), ty(bounds.GetLow().x, bounds.GetHigh().y));
    RenderEnd();
    
    // Rim Walls
    Color(1.0f, 1.0f, 1.0f, 0.5f);
    BeginLines();
    for (int i = se_rimWalls.Len() - 1; i >= 0; --i) {
        eWallRim *wall = se_rimWalls[i];
        if (wall) {
            Color(1.0f, 1.0f, 1.0f, 0.5f);
            Vertex(tx(wall->EndPoint(0).x, wall->EndPoint(0).y), ty(wall->EndPoint(0).x, wall->EndPoint(0).y));
            Vertex(tx(wall->EndPoint(1).x, wall->EndPoint(1).y), ty(wall->EndPoint(1).x, wall->EndPoint(1).y));
        }
    }
    RenderEnd();
    
    // --- Render Zones on Minimap ---
    const tList<eGameObject>& gameObjects = me->Object()->Grid()->GameObjects();
    float pulse = (sin(se_GameTime() * 4.0f) * 0.5f + 0.5f); // 0 to 1
    
    for (int j = gameObjects.Len() - 1; j >= 0; j--) {
        gZone *zone = dynamic_cast<gZone*>(gameObjects(j));
        if (zone) {
            eCoord zpos = zone->GetPosition();
            float zrad = zone->GetRadius();
            if (zrad > 0) {
                gRealColor zcol = zone->GetColor();
                
                // Pulsing inner fill (subtle, won't clash with player dots)
                glColor4f(zcol.r * 0.6f, zcol.g * 0.6f, zcol.b * 0.6f, 0.1f + 0.15f * pulse);
                BeginTriangleFan();
                Vertex(tx(zpos.x, zpos.y), ty(zpos.x, zpos.y));
                for (int i = 0; i <= 24; i++) {
                    float ang = i * 2.0f * M_PI / 24.0f;
                    float wx = zpos.x + cos(ang) * zrad;
                    float wy = zpos.y + sin(ang) * zrad;
                    Vertex(tx(wx, wy), ty(wx, wy));
                }
                RenderEnd();
                
                // Subtle border
                glColor4f(zcol.r * 0.7f, zcol.g * 0.7f, zcol.b * 0.7f, 0.5f);
                glLineWidth(1.5f);
                BeginLineLoop();
                for (int i = 0; i < 24; i++) {
                    float ang = i * 2.0f * M_PI / 24.0f;
                    float wx = zpos.x + cos(ang) * zrad;
                    float wy = zpos.y + sin(ang) * zrad;
                    Vertex(tx(wx, wy), ty(wx, wy));
                }
                RenderEnd();
            }
        }
    }
    // --- End Zones ---
    
    // Cycle Walls — render with proper tail clipping for WALLS_LENGTH
    BeginLines();

    // Helper lambda to draw a single wall segment with tail clipping
    auto drawWall = [&](gNetPlayerWall *wall) {
        if (!wall) return;
        gCycle *cycle = wall->Cycle();
        if (!cycle) return;

        eCoord p0 = wall->EndPoint(0); // tail (oldest point)
        eCoord p1 = wall->EndPoint(1); // head (newest point)

        REAL wallBeg = wall->BegPos();
        REAL wallEnd = wall->EndPos();

        // [MOD] Skip obsolete walls from previous round/life
        if (wallBeg > cycle->GetDistance()) return;

        // Skip walls with invalid coordinates outside arena bounds
        const eRectangle &bounds = eWallRim::GetBounds();
        float margin = 50.0f;
        if (p0.x < bounds.GetLow().x - margin || p0.x > bounds.GetHigh().x + margin ||
            p0.y < bounds.GetLow().y - margin || p0.y > bounds.GetHigh().y + margin ||
            p1.x < bounds.GetLow().x - margin || p1.x > bounds.GetHigh().x + margin ||
            p1.y < bounds.GetLow().y - margin || p1.y > bounds.GetHigh().y + margin) {
            return;
        }

        // Clip the tail if WALLS_LENGTH is active
        REAL maxLen = cycle->MaxWallsLength();
        if (maxLen > 0) {
            REAL visibleLen = cycle->ThisWallsLength();
            if (visibleLen <= 0.0f) return;
            REAL minVisibleDist = cycle->GetDistance() - visibleLen;

            // Skip entirely expired walls
            if (wallEnd < minVisibleDist) return;

            // Clip the start point if partially expired
            if (wallBeg < minVisibleDist && wallEnd > wallBeg) {
                REAL t = (minVisibleDist - wallBeg) / (wallEnd - wallBeg);
                if (t > 0.0f && t < 1.0f) {
                    p0.x = p0.x + (p1.x - p0.x) * t;
                    p0.y = p0.y + (p1.y - p0.y) * t;
                }
            }
        }

        // Fade dead cycle walls
        float alpha = 0.8f;
        if (!cycle->Alive()) alpha = 0.3f;

        Color(cycle->color_.r, cycle->color_.g, cycle->color_.b, alpha);
        Vertex(tx(p0.x, p0.y), ty(p0.x, p0.y));
        Vertex(tx(p1.x, p1.y), ty(p1.x, p1.y));
    };

    for (int i = sg_netPlayerWalls.Len() - 1; i >= 0; --i) {
        drawWall(sg_netPlayerWalls[i]);
    }
    for (int i = sg_netPlayerWallsGridded.Len() - 1; i >= 0; --i) {
        drawWall(sg_netPlayerWallsGridded[i]);
    }
    RenderEnd();
    
    // Players (using quads) — only alive
    BeginQuads();
    for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; --i) {
        ePlayerNetID *p = se_PlayerNetIDs(i);
        if (p && p->Object() && p->Object()->Alive()) {
            gCycle *cycle = dynamic_cast<gCycle*>(p->Object());
            if (cycle) {
                Color(cycle->color_.r, cycle->color_.g, cycle->color_.b, 1.0f);
                float screen_x = tx(cycle->Position().x, cycle->Position().y);
                float screen_y = ty(cycle->Position().x, cycle->Position().y);
                float s_y = 0.003f * sg_modMinimapScale;
                float s_x = s_y / aspect; 
                Vertex(screen_x - s_x, screen_y - s_y);
                Vertex(screen_x + s_x, screen_y - s_y);
                Vertex(screen_x + s_x, screen_y + s_y);
                Vertex(screen_x - s_x, screen_y + s_y);
            }
        }
    }
    RenderEnd();
    
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_TEXTURE_2D);
}



REAL subby_SpeedGaugeSize=.175, subby_SpeedGaugeLocX=0.0, subby_SpeedGaugeLocY=-0.9;
REAL subby_BrakeGaugeSize=.175, subby_BrakeGaugeLocX=0.48, subby_BrakeGaugeLocY=-0.9;
REAL subby_RubberGaugeSize=.175, subby_RubberGaugeLocX=-0.48, subby_RubberGaugeLocY=-0.9;
bool subby_ShowHUD=true, subby_ShowSpeedFastest=true, subby_ShowScore=true, subby_ShowAlivePeople=true, subby_ShowPing=true, subby_ShowSpeedMeter=true, subby_ShowBrakeMeter=true, subby_ShowRubberMeter=true;
bool showTime=false;
bool show24hour=false;
REAL subby_ScoreLocX=-0.95, subby_ScoreLocY=-0.85, subby_ScoreSize =.13;
REAL subby_FastestLocX=-0.2, subby_FastestLocY=-0.95, subby_FastestSize =.13;
REAL subby_AlivePeopleLocX=.45, subby_AlivePeopleLocY=-0.95, subby_AlivePeopleSize =.13;
REAL subby_PingLocX=.80, subby_PingLocY=-0.95, subby_PingSize =.13;

REAL max_player_speed=0;

namespace
{
static tConfigMigration migrate([](tString const &savedInVersion){
    // defaults for subby_FastestSize and subby_FastestLocX changed on 0.2.9.1
    if(tConfigMigration::SavedBefore(savedInVersion, "0.2.9.1_alpha"))
    {
        if(subby_FastestSize < subby_AlivePeopleSize)
            subby_FastestSize = .13; // was .12
        if(subby_FastestLocX > -.1)
            subby_FastestLocX = -.2; // was -0.0
    }
});
}

#ifndef DEDICATED

void GLmeter_subby(float value,float max, float locx, float locy, float size, const char * t,bool displayvalue = true, bool reverse = false, REAL r=.5, REAL g=.5, REAL b=1)
{

#ifndef DEDICATED
    if (!sr_glOut)
        return;
#endif

    tString title( t );

    float x, y;
    char string[50];
    value>max?value=max:1;
    value<0?value=0:1;
    x= (reverse?-1:1) * cos(M_PI*value/max);
    y= sin(M_PI*value/max);
    if(y<size*.24) displayvalue = false; // dont display text when at minimum

    /* Draws an ugly background on the gauge
    	BeginQuads();
    	Color(1.,1.,1.,.8);
    	Vertex(locx-size-.04,locy-.04,0);
    	Vertex(locx-size-.04,locy+size+.04,0);
    	Vertex(locx+size+.04,locy+size+.04,0);
    	Vertex(locx+size+.04,locy-.04,0);

    	Color(.1,.1,.1,.8);
    	Vertex(locx-size-.02,locy-.02,0);
    	Vertex(locx-size-.02,locy+size+.02,0);
    	Vertex(locx+size+.02,locy+size+.02,0);
    	Vertex(locx+size+.02,locy-.02,0);

    	RenderEnd();*/

    glDisable(GL_TEXTURE_2D);
    Color(r,g, b);
    BeginLines();
    Vertex(-.1*x*size+locx,.1*y*size+locy,0);
    Vertex(-x*size+locx,y*size+locy,0);
    RenderEnd();


    rTextField min_t(-size-(0.1*size)+locx,locy,.12*size,.24*size);
    rTextField max_t(+size+(0.1*size)+locx,locy,.12*size,.24*size);
    min_t << "0xcccccc" << (reverse?max:0);
    max_t << "0xcccccc" << (reverse?0:max);

    if(displayvalue){
        rTextField speed_t(-x*1.45*size+locx,y*1.35*size+locy,.12*size,.24*size);
        sprintf(string,"%.1f",value);
        speed_t << "0xccffff" << (string);
    }
    int length = title.Len();
    if ( length > 0 )
    {
        rTextField titletext(locx-((.15*size*(length-1.5))/2.0),locy,.12*size,.24*size); //centre  -1.0 for null char and -.5 for half a char width = -1.5
        titletext << "0xff3333" << title;
    }
}

static REAL sg_hudCacheThreshold=.0001;
static tSettingItem< REAL > sg_minDropIntervalConf( "HUD_CACHE_THRESHOLD", sg_hudCacheThreshold );

class gGLMeter
{
public:
    gGLMeter()
    : oldTime_( -100 ), oldRel_( -100 )
    {
    }

    void Display( float value,float max, float locx, float locy, float size, const char * t,bool displayvalue = true, bool reverse = false, REAL r=.5, REAL g=.5, REAL b=1)
    {
        REAL rel = value/max;
        REAL time = se_GameTime();
        REAL change = rel - oldRel_;

        // see if the gauge change is enough to warrant an update
        if ( change * change * ( time - oldTime_ ) > sg_hudCacheThreshold*sg_hudCacheThreshold || time < oldTime_ )
        {
            list_.Clear();
        }

        if ( !list_.Call() )
        {
            oldRel_ = rel;
            oldTime_ = time;

            rDisplayListFiller filler( list_ );
            GLmeter_subby(value, max,  locx, locy, size, t, displayvalue, reverse, r, g, b );
        }
    }
private:
    REAL oldTime_;      // last rendered game time
    REAL oldRel_;       // last rendered gauge position
    rDisplayList list_; // caching display list
};

// caches stuff based on two float properties
class gTextCache
{
public:
    gTextCache()
    : propa_(-1), propb_(-1)
    {
    }

    bool Call( REAL propa, REAL propb )
    {
        if ( propa != propa_ || propb != propb_ )
        {
            propa_ = propa;
            propb_ = propb;
            list_.Clear();
            return false;
        }
        else
        {
            return list_.Call();
        }
    }
    
    rDisplayList list_;
private:
    REAL propa_, propb_;
};

static int alivepeople, alivemates, thetopscore, hudfpscount;

static void tank_display_hud( ePlayerNetID* me ){
    static int fps       = 60;
    static REAL lastTime = 0;

    if (se_mainGameTimer &&
            se_mainGameTimer->speed > .9 &&
            se_mainGameTimer->speed < 1.1 &&
            se_mainGameTimer->IsSynced() )
    {
        REAL newtime = tSysTimeFloat();
        REAL ts      = newtime - lastTime;
        int newfps   = static_cast<int>(se_FPS());
        if (fabs((newfps-fps)*ts)>4)
        {
            fps      = newfps;
            lastTime = newtime;
        }

        Color(1,1,1);
        rTextField c(-.9,-.6);

        // signed short int playerID = -1;
        //	  unsigned short int alivepeople2 = 0;
        unsigned short int max = se_PlayerNetIDs.Len();
        //	  signed short int scores[16];
        //	  signed short int teamID = -1;

        bool dohudcrap = false;
        if (hudfpscount >= fps) {
            hudfpscount = 0;
            dohudcrap = true;
        }
        else { hudfpscount++; }


        //Calculation, enemies (people not on me's team) alive
        if (dohudcrap) {
            alivepeople=0;
            alivemates=0;
            if (me){
                for(unsigned short int i=0;i<max;i++){
                    ePlayerNetID *p=se_PlayerNetIDs(i);
                    if (p->Object() && p->Object()->Alive()){
                        if (p->CurrentTeam() != me->CurrentTeam()){
                            alivepeople++;
                        }else{
                            alivemates++;
                        }
                    }
                }
            }
        }

        /*	  int max = teams.Len();
        	  for(int i=0;i<max;i++){
        	    eTeam *t = teams(i);
        	  }*/

        if ( me ){
            //Calculation, top player score
            if (dohudcrap) {
                //	    unsigned short int maxscore = 0;
                thetopscore = 0;
                for(unsigned short int i=0;i<max;i++){
                    ePlayerNetID* p2=se_PlayerNetIDs(i);
                    //	  if (p2->TotalScore() > p->TotalScore() || p2->TotalScore() > thetopscore){
                    if (p2->TotalScore() > thetopscore){
                        thetopscore = p2->TotalScore();
                    }
                }
            }

            eCoord pcpos; // = 0 implied
            if (me->Object() && me->Object()->Alive()) {
                pcpos = me->Object()->Position(); }

            tString line1, line2, line3;

            /*    line1 << "HUD: " << p->name;
              line1.SetPos(22, true);
                    line1 << "FPS: " << fps;
              line1.SetPos(34, true);
              line1 << "ENEMIES: " << alivepeople << "\n";

                 line2 << "SCORE: " << p->TotalScore() << ", " << thetopscore;
              line2.SetPos(22, true);
                 line2 << "PING: " << int(p->ping*1000) << "\n";

              line3 << "LOCATION: " << int(pcpos.x);
              line3 << ", " << int(pcpos.y);
              line3.SetPos(22, true);
                // line3 << "RUBBER: " << int(p->rubberstatus*100);


             c <<line1 << line2; */


        }
    }
}

static void display_hud_subby( ePlayer* player ){
    if ( !player )
        return;
    ePlayerNetID *me = player->netPlayer;

    if (se_mainGameTimer &&
            se_mainGameTimer->speed > .9 &&
            se_mainGameTimer->speed < 1.1 &&
            se_mainGameTimer->IsSynced() )
    {
        Color(1,1,1);
        rTextField c(-.9,-.85);
        rTextField t(.6,.98);

        // t << "0xffffffGame Time:" << int (se_GameTime())<< " s";
        if(subby_ShowHUD){
            // static int max=0,lastmax=0;
            // static int imax=-1,lastimax=0;
            // static REAL imax=-1;
            char fasteststring[50];
            // REAL distance;
            static float maxmeterspeed= 50;
            float myping;
            static REAL max=max_player_speed;
            static tString name;
            static tString ultiname;
            // static bool  wrotefastest = false;
            // static bool wrote10 =false;
            static bool belowzero=false;
            static REAL ultimax=0;
            // static REAL timelast;
            static REAL myscore, topscore;
            // tString realname = ePlayer::PlayerConfig(0)->Name();

            // const tString& myname = me->name;

            if ( me )
                myscore = me->TotalScore();

            const eGameObject *ret = NULL;
            if(player->cam){
                ret = player->cam->Center();

                static bool firsttime =true;

                if(se_GameTime()<0||!firsttime){
                    firsttime=false;
                    if((se_GameTime()<0)){
                        belowzero=true;
                        if(se_GameTime()<-1){
                            tOutput message;
                            //  message<< "Last Game Fastest: " << name << " with a top speed of " << max << "\n";
                            //  con << "testcon";
                            //  sn_CenterMessage(message);
                        }else{
                            // timelast = se_GameTime();
                            max=0;
                            // wrotefastest = false;
                            // wrote10 =false;
                        }
                    }else if(se_GameTime()>0&&belowzero){
                        belowzero=false;
                    }
                }

                topscore =0;
                for(int i =0 ; i< se_PlayerNetIDs.Len(); i++){
                    ePlayerNetID *p = se_PlayerNetIDs[i];
                    p->TotalScore()>topscore?topscore=p->TotalScore():1;

                    if(gCycle *h = (gCycle*)(p->Object())){

                        // change player so we always see the gauges of the cycle that is watched
                        if ( h == ret )
                            me = p;

                        //   distance = h->distance;
                        //   c << p-> name << " " << int((h ->Speed())) << " ";
                        if (h->Speed()>max){
                            max =  (float) h->Speed();  // changed to float for more accuracy in reporting top speed
                            name = p->GetName();
                            // imax = i;

                        }
                        if( h->Speed()>ultimax){
                            ultimax = (float) h->Speed();
                            ultiname = p->GetName();
                        }
                    }
                }

                if (me!=NULL){
                    gCycle *h = dynamic_cast<gCycle *>(me->Object());
                    if (h && ( !player->netPlayer || !player->netPlayer->IsChatting()) && se_GameTime()>-2){
                        h->Speed()>maxmeterspeed?maxmeterspeed+=10:1;
                        // myscore=p->TotalScore();
                        myping = me->ping;

                        extern bool sg_modSpeedometerEnabled;
                        extern bool sg_modRubberMeterEnabled;
                        extern bool sg_modBrakeMeterEnabled;

                        if(subby_ShowSpeedMeter && !sg_modSpeedometerEnabled)
                        {
                            static gGLMeter meter[MAX_PLAYERS];
                            meter[player->ID()].Display(h->Speed(),maxmeterspeed,subby_SpeedGaugeLocX,subby_SpeedGaugeLocY,subby_SpeedGaugeSize,"Speed");  // easy to use configurable meters
                        }
                        if(subby_ShowRubberMeter && !sg_modRubberMeterEnabled)
                        {
                            static gGLMeter meter[MAX_PLAYERS];
                            meter[player->ID()].Display(h->GetRubber(),sg_rubberCycle,subby_RubberGaugeLocX,subby_RubberGaugeLocY,subby_RubberGaugeSize," Rubber Used");
                            if ( gCycle::RubberMalusActive() )
                            {
                                static gGLMeter meter2[MAX_PLAYERS];
                                meter2[player->ID()].Display(100/(1+h->GetRubberMalus()),100,subby_RubberGaugeLocX,subby_RubberGaugeLocY,subby_RubberGaugeSize,"",true, false, 1,.5,.5);
                            }
                        }
                        if(subby_ShowBrakeMeter && !sg_modBrakeMeterEnabled)
                        {
                            static gGLMeter meter[MAX_PLAYERS];
                            meter[player->ID()].Display(h->GetBrakingReservoir(), 1.0,subby_BrakeGaugeLocX,subby_BrakeGaugeLocY,subby_BrakeGaugeSize, " Brakes");
                        }


                        //  bool displayfastest = true;// put into global, set via menusytem... subby to do.make sr_DISPLAYFASTESTout

                        if(subby_ShowSpeedFastest)
                        {
                            static gTextCache cacheArray[MAX_PLAYERS];
                            gTextCache & cache = cacheArray[player->ID()];
                            if ( !cache.Call( max, 0 ) )
                            {
                                rDisplayListFiller filler( cache.list_ );

                                float size= subby_FastestSize;
                                tColoredString message,messageColor;
                                messageColor << "0xbf9d50";

                                sprintf(fasteststring,"%.1f",max);
                                message << "  Fastest: " << name << " " << fasteststring;
                                message.RemoveHex(); //cheers tank;
                                int length = message.Len();

                                rTextField speed_fastest(subby_FastestLocX-((.15*size*(length-1.5))/2.0),subby_FastestLocY,.15*size,.3*size);
                            /*   rTextField speed_fastest(.7-((.15*size*(length-1.5))/2.0),.65,.15*size,.3*size); */

                                speed_fastest << messageColor << message;
                            }
                        }

                        if(subby_ShowScore){
                            static gTextCache cacheArray[MAX_PLAYERS];
                            gTextCache & cache = cacheArray[player->ID()];
                            if ( !cache.Call( topscore, myscore ) )
                            {
                                rDisplayListFiller filler( cache.list_ );

                                tString colour;
                                if(myscore==topscore){
                                    colour = "0xff9d50";
                                }else if (myscore > topscore){
                                    colour = "0x11ff11";
                                }else{
                                    colour = "0x11ffff";
                                }

                                float size = subby_ScoreSize;
                                rTextField score(subby_ScoreLocX,subby_ScoreLocY,.15*size,.3*size);
                                score <<               " Scores\n";
                                score << "0xefefef" << "Me:  Top:\n";
                                score << colour << myscore << "     0xffff00" << topscore ;
                            }
                        }

                        if(subby_ShowAlivePeople){
                            static gTextCache cacheArray[MAX_PLAYERS];
                            gTextCache & cache = cacheArray[player->ID()];
                            if ( !cache.Call( alivepeople, alivemates ) )
                            {
                                rDisplayListFiller filler( cache.list_ );

                                tString message;
                                message << "Enemies: " << alivepeople << " Friends: " << alivemates;
                                int length = message.Len();
                                float size = subby_AlivePeopleSize;
                                rTextField enemies_alive(subby_AlivePeopleLocX-((.15*size*(length-1.5))/2.0),subby_AlivePeopleLocY,.15*size,.3*size);
                                enemies_alive << "0xfefefe" << message;
                            }
                        }

                        if(subby_ShowPing){
                            static gTextCache cacheArray[MAX_PLAYERS];
                            gTextCache & cache = cacheArray[player->ID()];
                            if ( !cache.Call( 0, myping ) )
                            {
                                rDisplayListFiller filler( cache.list_ );

                                tString message;
                                message << "Ping: " << int(myping * 1000) << " ms" ;
                                int length = message.Len();
                                float size = subby_PingSize;
                                rTextField ping(subby_PingLocX-((.15*size*(length-1.5))/2.0),subby_PingLocY,.15*size,.3*size);
                                ping << "0xfefefe" << message;
                            }
                        }
                    }
                }
            }
        }

    }
    tank_display_hud( me ); // call tank's version

}

static void display_fps_subby()
{
    if (!(se_mainGameTimer &&
            se_mainGameTimer->speed > .9 &&
            se_mainGameTimer->speed < 1.1 &&
            se_mainGameTimer->IsSynced() ) )
        return;

    static int fps       = 60;
    static REAL lastTime = 0;

    REAL newtime = tSysTimeFloat();
    REAL ts      = newtime - lastTime;

    static gTextCache cache;
    if ( cache.Call( fps, (int)tSysTimeFloat() ) )
    {
        return;
    }
    if ( tRecorder::IsRunning() )
    {
        cache.list_.Clear();
    }
    rDisplayListFiller filler( cache.list_ );

    float size =.15;
    rTextField c2(.9-.2*rTextField::AspectWidthMultiplier(),.85,.15*size*rTextField::AspectWidthMultiplier(), .3*size);

    if ( sr_RecordingTimeOut && tRecorder::IsRunning() )
    {
        std::stringstream time;
        time.precision( 5 );
        time << newtime;
        {
            c2 << "0xffffffT:" << time.str() << "\n";
        }
    }

    int newfps   = static_cast<int>(se_FPS());
    if (fabs((newfps-fps)*ts)>4)
    {
        fps      = newfps;
        lastTime = newtime;
    }

    if(sr_FPSOut){
        c2 << "0xffffffFPS: " <<fps;
    }
    // Show the time
    if(showTime) {
        static int lastTime=0;
        static char theTime[13*3];
        float size =.15;
        struct tm* thisTime;
        time_t rawtime;

        time ( &rawtime );
        thisTime = localtime ( &rawtime );

        if(thisTime->tm_min != lastTime) {
            char h[13];
            char m[13];
            char ampm[13] = " ";

            lastTime = thisTime->tm_min;

            if(thisTime->tm_min < 10) {
                sprintf(m, "0%d", thisTime->tm_min);
            } else {
                sprintf(m, "%d", thisTime->tm_min);
            }

            if(show24hour) {
                sprintf(h, "%d", thisTime->tm_hour);
            } else {
                int newhour;

                if(thisTime->tm_hour > 12) {
                    newhour = thisTime->tm_hour-12;
                    sprintf(ampm,"%s","PM");
                } else {
                    newhour = thisTime->tm_hour;
                    if(newhour == 0) newhour = 12;
                    sprintf(ampm,"%s","AM");
                }
                sprintf(h, "%d", newhour);
            }

            sprintf(theTime, "%s:%s %s",h,m,ampm);
        }

        // float timesize = subby_ScoreSize; // -((.15*timesize*(10-1.5))/2.0)
        rTextField the_time(.9-.2*rTextField::AspectWidthMultiplier(),.9,.15*size*rTextField::AspectWidthMultiplier(), .3*size);

        the_time << "0xffffff" << theTime;
    }

    // [MOD] K/D Ratio Counter — tracked always, rendered via modern HUD
    // Tracks enemy deaths + score changes for accurate kill/death detection
    {
        static int sg_kdLastScore = -1;
        static bool sg_kdWasAlive = false;
        static REAL sg_kdLastGameTime = -100.0f;
        // Track all player alive states for kill detection
        static std::map<int, bool> sg_kdPlayerAlive;

        if (sg_modKDResetFlag) {
            sg_kdKills = 0; sg_kdDeaths = 0;
            sg_kdLastScore = -1;
            sg_kdWasAlive = false;
            sg_kdLastGameTime = -100.0f;
            sg_kdPlayerAlive.clear();
            sg_modKDResetFlag = false;
        }

        ePlayer* kp = ePlayer::PlayerConfig(0);
        if (kp && kp->netPlayer) {
            bool alive = kp->netPlayer->Object() && kp->netPlayer->Object()->Alive();
            int score = kp->netPlayer->Score();
            int myNetID = kp->netPlayer->ID();
            REAL gameTime = se_GameTime();

            // --- Kill detection ---
            // Check for enemy deaths when we WERE alive (covers trade kills where
            // we die in the same frame as the enemy)
            bool someEnemyDied = false;
            if (sg_kdWasAlive && gameTime > 0.5f) {
                for (int ei = 0; ei < se_PlayerNetIDs.Len(); ei++) {
                    ePlayerNetID *ep = se_PlayerNetIDs(ei);
                    if (!ep || ep->ID() == myNetID) continue;
                    // Same team = skip (no teamkill points)
                    if (ep->CurrentTeam() && kp->netPlayer->CurrentTeam() &&
                        ep->CurrentTeam() == kp->netPlayer->CurrentTeam()) continue;
                    int eid = ep->ID();
                    bool eAlive = ep->Object() && ep->Object()->Alive();
                    if (sg_kdPlayerAlive.count(eid) && sg_kdPlayerAlive[eid] && !eAlive) {
                        someEnemyDied = true;
                    }
                    sg_kdPlayerAlive[eid] = eAlive;
                }
            }

            // Count kill: score increased AND enemy died (while we were alive this frame or last)
            if (sg_kdLastScore >= 0 && score > sg_kdLastScore && someEnemyDied) {
                sg_kdKills++;
            }

            // --- Death detection ---
            // Count death when we were alive and now aren't, AND the previous frame
            // was during active gameplay (sg_kdLastGameTime > 0.5).
            // We DON'T check current gameTime — when you're alone and suicide,
            // the round resets instantly and gameTime jumps negative in the same frame.
            // By checking only LAST frame's gameTime, we catch that death.
            if (sg_kdWasAlive && !alive && sg_kdLastGameTime > 0.5f) {
                sg_kdDeaths++;
            }

            // Clear tracking when game resets to avoid stale data
            if (gameTime < 0.0f && sg_kdLastGameTime > 0.0f) {
                sg_kdPlayerAlive.clear();
                sg_kdWasAlive = false;
            }

            sg_kdLastScore = score;
            sg_kdWasAlive = alive;
            sg_kdLastGameTime = gameTime;
        }
    }
}

// ============ [MOD] Teammate Death Warning ============
static std::map<int, bool> sg_wasAliveMap; // network ID -> was alive last frame
static std::map<int, REAL> sg_deathTimeMap; // network ID -> time when first detected dead
static std::map<int, bool> sg_alertTriggeredMap; // network ID -> alert already triggered for this death
REAL sg_teammateDeathFlashTime = -100.0f;
tString sg_deadTeammateName;

static void display_teammate_death_warning(ePlayerNetID* me)
{
    if (!sg_modTeammateDeathWarning || !me || !me->CurrentTeam())
        return;

    REAL gameTime = se_GameTime();

    // Reset maps during countdown/between rounds to avoid stale data
    if (gameTime < 0.5f) {
        sg_wasAliveMap.clear();
        sg_deathTimeMap.clear();
        sg_alertTriggeredMap.clear();
    }

    // Detect teammate death by tracking alive states via map (safe for any net ID)
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
        ePlayerNetID *p = se_PlayerNetIDs(i);
        if (!p) continue;

        int id = p->ID();
        bool isAlive = p->Object() && p->Object()->Alive();

        if (isAlive) {
            sg_wasAliveMap[id] = true;
            sg_deathTimeMap[id] = -1.0f;
            sg_alertTriggeredMap[id] = false;
        } else {
            // Player is currently dead/inactive
            if (!sg_wasAliveMap.count(id) || sg_wasAliveMap[id]) {
                // Just transitioned from alive to dead/inactive
                sg_wasAliveMap[id] = false;
                sg_deathTimeMap[id] = gameTime;
                sg_alertTriggeredMap[id] = false;
            }

            // Only trigger if they have been dead/inactive for at least 0.1 seconds,
            // which filters out 1-2 frame prediction rollbacks during kills/collisions
            if (!sg_alertTriggeredMap[id] && sg_deathTimeMap[id] > 0.0f) {
                if (gameTime - sg_deathTimeMap[id] >= 0.1f) {
                    if (p->CurrentTeam() == me->CurrentTeam() && p != me && gameTime > 1.0f) {
                        sg_teammateDeathFlashTime = gameTime;
                        sg_deadTeammateName = p->GetName();
                    }
                    sg_alertTriggeredMap[id] = true;
                }
            }
        }
    }
}

// ============ [MOD] Fortress Zone Alerts ============
static void display_fortress_alerts(ePlayerNetID* me)
{
    return;

    eTeam* myTeam = me->CurrentTeam();
    eGrid* grid = me->Object()->Grid();
    if (!grid) return;

    const tList<eGameObject>& gameObjects = grid->GameObjects();

    bool ourZoneAttacked = false;
    bool enemyZoneCapturing = false;
    REAL ourWorstConquest = 0.0f;
    REAL enemyBestConquest = 0.0f;

    // Team color of our team (RGB 0-15 range in engine, normalize)
    float myR = myTeam->R() / 15.0f;
    float myG = myTeam->G() / 15.0f;
    float myB = myTeam->B() / 15.0f;

    for (int j = gameObjects.Len() - 1; j >= 0; j--) {
        gZone *zone = dynamic_cast<gZone*>(gameObjects(j));
        if (!zone) continue;

        // Use rotation speed as proxy for conquest activity
        REAL rotSpeed = zone->GetRotationSpeed();
        REAL baseRot = 0.3f;
        if (rotSpeed <= baseRot + 0.1f) continue; // idle zone, skip

        REAL urgency = (rotSpeed - baseRot) / 10.0f;
        if (urgency > 1.0f) urgency = 1.0f;

        // Determine if zone belongs to our team by matching color
        gRealColor const &zc = zone->GetColor();
        float dr = zc.r - myR, dg = zc.g - myG, db = zc.b - myB;
        float colorDist = dr*dr + dg*dg + db*db;

        if (colorDist < 0.3f) {
            // Our zone — being attacked
            ourZoneAttacked = true;
            if (urgency > ourWorstConquest) ourWorstConquest = urgency;
        } else {
            // Enemy zone — we might be conquering
            enemyZoneCapturing = true;
            if (urgency > enemyBestConquest) enemyBestConquest = urgency;
        }
    }

    if (!ourZoneAttacked && !enemyZoneCapturing) return;

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float pulse = 0.5f + 0.5f * sinf((float)tSysTimeFloat() * 6.0f);

    if (ourZoneAttacked) {
        float barAlpha = 0.15f + 0.15f * pulse;
        glColor4f(1.0f, 0.1f, 0.1f, barAlpha);
        BeginQuads();
        Vertex(-1.0f, -0.5f); Vertex(-0.96f, -0.5f);
        Vertex(-0.96f, 0.5f); Vertex(-1.0f, 0.5f);
        RenderEnd();

        glEnable(GL_TEXTURE_2D);
        float tSz = 0.05f, tW = tSz * rTextField::AspectWidthMultiplier();
        rTextField alert(-0.94f, 0.40f, tW, tSz * 1.6f);
        alert << "0xff3333" << "ZONE ATTACK";
        glDisable(GL_TEXTURE_2D);

        // Progress bar
        float bL = -0.94f, bR = bL + 0.25f * ourWorstConquest;
        glColor4f(0.3f, 0.0f, 0.0f, 0.4f);
        BeginQuads();
        Vertex(bL, 0.30f); Vertex(bL+0.25f, 0.30f);
        Vertex(bL+0.25f, 0.33f); Vertex(bL, 0.33f);
        RenderEnd();
        glColor4f(1.0f, 0.2f, 0.1f, 0.7f);
        BeginQuads();
        Vertex(bL, 0.30f); Vertex(bR, 0.30f);
        Vertex(bR, 0.33f); Vertex(bL, 0.33f);
        RenderEnd();
    }

    if (enemyZoneCapturing) {
        float barAlpha = 0.15f + 0.15f * pulse;
        glColor4f(0.1f, 1.0f, 0.2f, barAlpha);
        BeginQuads();
        Vertex(0.96f, -0.5f); Vertex(1.0f, -0.5f);
        Vertex(1.0f, 0.5f); Vertex(0.96f, 0.5f);
        RenderEnd();

        glEnable(GL_TEXTURE_2D);
        float tSz = 0.05f, tW = tSz * rTextField::AspectWidthMultiplier();
        rTextField alert(0.58f, 0.40f, tW, tSz * 1.6f);
        alert << "0x33ff44" << "CONQUERING";
        glDisable(GL_TEXTURE_2D);

        float bL = 0.58f, bR = bL + 0.25f * enemyBestConquest;
        glColor4f(0.0f, 0.3f, 0.0f, 0.4f);
        BeginQuads();
        Vertex(bL, 0.30f); Vertex(bL+0.25f, 0.30f);
        Vertex(bL+0.25f, 0.33f); Vertex(bL, 0.33f);
        RenderEnd();
        glColor4f(0.1f, 1.0f, 0.2f, 0.7f);
        BeginQuads();
        Vertex(bL, 0.30f); Vertex(bR, 0.30f);
        Vertex(bR, 0.33f); Vertex(bL, 0.33f);
        RenderEnd();
    }

    glEnable(GL_TEXTURE_2D);
}

// ============ [MOD] Live Scoreboard Overlay ============
static void display_scoreboard_overlay(ePlayerNetID* me)
{
    return; // Bypassed in favor of modern ImGui LiveScoreboardWidget

    // Build flat player list from all teams, sorted by team score then player score
    struct Entry {
        tColoredString name;
        int score;
        bool alive;
        int teamScore;
        unsigned short tR, tG, tB;
    };
    std::vector<Entry> entries;

    for (int i = 0; i < eTeam::teams.Len(); i++) {
        eTeam* t = eTeam::teams(i);
        if (!t || t->NumPlayers() == 0) continue;
        for (int j = 0; j < t->NumPlayers(); j++) {
            ePlayerNetID* p = t->Player(j);
            if (!p) continue;
            Entry e;
            e.name = p->GetColoredName();
            e.score = p->Score();
            e.alive = p->Object() && p->Object()->Alive();
            e.teamScore = t->Score();
            e.tR = t->R(); e.tG = t->G(); e.tB = t->B();
            entries.push_back(e);
        }
    }

    // Sort: by team score desc, then by player score desc within same team
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        if (a.teamScore != b.teamScore) return a.teamScore > b.teamScore;
        return a.score > b.score;
    });

    if (entries.empty()) return;

    int count = (int)entries.size();
    if (count > 16) count = 16;

    // Layout
    float sz   = 0.022f;
    float aspW = rTextField::AspectWidthMultiplier();
    float cW   = sz * aspW;
    float rowH = sz * 2.0f;
    float totalH = count * rowH;
    float xPos = -0.92f;
    float yStart = totalH * 0.5f;  // vertically centered

    for (int i = 0; i < count; i++) {
        Entry& e = entries[i];
        float y = yStart - i * rowH;

        // Dim dead players
        const char* dimPre  = e.alive ? "" : "0x555555";
        const char* dimPost = e.alive ? "0xaaaaaa" : "0x444444";

        rTextField row(xPos, y, cW, rowH);
        if (!e.alive) row << "0x555555";
        row << e.name << " " << dimPost << e.score;
    }
}

// [MOD] Cinematic mode — defined in eCamera.cpp
extern bool sg_noclipCinematic;

static void display_hud_subby_all()
{
    // [MOD] Cinematic mode: skip ALL HUD rendering for clean screen
    if (sg_noclipCinematic) return;

    sr_ResetRenderState(true);
    display_fps_subby();


    rViewportConfiguration* viewportConfiguration = rViewportConfiguration::CurrentViewportConfiguration();

    for ( int viewport = viewportConfiguration->num_viewports-1; viewport >= 0; --viewport )
    {
        // get the ID of the player in the viewport
        int playerID = sr_viewportBelongsToPlayer[ viewport ];

        // get the player
        ePlayer* player = ePlayer::PlayerConfig( playerID );

        // select the full viewport for the minimap (bypassing HUD aspect ratio)
        viewportConfiguration->Port( viewport )->Select();
        if (player && player->netPlayer && sg_modMinimapEnabled && false) {
            display_minimap_subby( player->netPlayer );
        }

        // select the corrected viewport for the rest of the HUD
        viewportConfiguration->Port( viewport )->CorrectAspectBottom().Select();

        // delegate
        display_hud_subby( player );

        // [MOD] Teammate death warning
        if (player && player->netPlayer) {
            display_teammate_death_warning( player->netPlayer );
        }

        // [MOD] Fortress zone alerts
        if (player && player->netPlayer) {
            display_fortress_alerts( player->netPlayer );
        }

        // [MOD] Live scoreboard overlay
        if (player && player->netPlayer) {
            display_scoreboard_overlay( player->netPlayer );
        }

        // [MOD] Zone Timers — use global grid, works alive or dead
        if (false && sg_modZoneTimers) {
            eGrid* grid = eGrid::CurrentGrid();
            if (grid) {
                const tList<eGameObject>& gameObjects = grid->GameObjects();
                float bestTime = 9999.0f;
                float bestRad = 0.0f;
                float bestSpeed = 0.0f;
                int zoneCount = 0;
                for (int j = gameObjects.Len() - 1; j >= 0; j--) {
                    gZone *zone = dynamic_cast<gZone*>(gameObjects(j));
                    if (zone) {
                        zoneCount++;
                        float zrad = zone->GetRadius();
                        float zspeed = zone->GetExpansionSpeed();
                        if (zrad > 0.0f) {
                            if (zrad > bestRad) {
                                bestRad = zrad;
                                bestSpeed = zspeed;
                            }
                            if (zspeed < -0.001f) {
                                float timeLeft = zrad / -zspeed;
                                if (timeLeft < bestTime) bestTime = timeLeft;
                            }
                        }
                    }
                }
                // Always show zone info if any zone exists
                if (zoneCount > 0) {
                    char timeStr[64];
                    if (bestTime < 9999.0f)
                        snprintf(timeStr, sizeof(timeStr), "Zone: %.1fs", bestTime);
                    else if (bestRad > 0.0f)
                        snprintf(timeStr, sizeof(timeStr), "Zone R:%.0f S:%.1f", bestRad, bestSpeed);
                    else
                        snprintf(timeStr, sizeof(timeStr), "Zones: %d", zoneCount);
                    float aspectW = rTextField::AspectWidthMultiplier();
                    rTextField timerField(-0.15f * aspectW, 0.45f, 0.04f * aspectW, 0.08f);
                    if (bestTime < 3.0f) timerField << "0xff3333";
                    else if (bestTime < 10.0f) timerField << "0xffff33";
                    else timerField << "0x33ff33";
                    timerField << timeStr;
                }
            }
        }

    }
}

static rPerFrameTask dfps(&display_hud_subby_all);
#endif // DEDICATED
