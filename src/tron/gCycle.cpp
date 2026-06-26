

#include "gCycle.h"
#include "TrailRenderer.h"
#ifndef DEDICATED
#include "DemoPlayer.h"
#endif
#include "nConfig.h"
#include "rModel.h"

#include "eGrid.h"
#include "rTexture.h"
#include "eTimer.h"
#include "tInitExit.h"
#include "tRecorder.h"
#include "rScreen.h"
#include "rFont.h"
#include "gSensor.h"
#include "ePlayer.h"
#include "eSound.h"
#include "eGrid.h"
#include "eFloor.h"
#include "gSparks.h"
#include "gExplosion.h"
#include "gWall.h"
#include "nKrawall.h"
#include "gAIBase.h"
#include "eDebugLine.h"
#include "eLagCompensation.h"
#include "gArena.h"
#include "gMenus.h"
#include "VisualsManager.h"
#ifndef DEDICATED
#include "DemoRecorder.h"
#endif

#include "tConsole.h"

#include "tMath.h"
#include <stdlib.h>
#include <fstream>
#include <memory>

#ifndef DEDICATED
#define DONTDOIT
#include "rRender.h"
#endif


#include "tDirectories.h"

#include <cmath>

class gRubberWaypointObject : public eReferencableGameObject {
public:
    gRubberWaypointObject(eGrid *grid, const eCoord &pos, REAL amount, gRealColor color) 
        : eReferencableGameObject(grid, pos, eCoord(1,0), NULL, true), amount_(amount), color_(color) {
        creationTime_ = se_GameTime();
        AddToList();
    }
    virtual ~gRubberWaypointObject() {}

    virtual bool Timestep(REAL currentTime) {
        
        if (currentTime - creationTime_ > sg_modWaypointsLifetime) {
            return true; 
        }
        return false; 
    }
    
#ifndef DEDICATED
    virtual void Render(const eCamera *cam) {
        if (!cam->RenderingMain()) return;
        
        REAL age = se_GameTime() - creationTime_;
        REAL alpha = 1.0f;
        REAL fadeStart = sg_modWaypointsLifetime - 2.0f;
        if (fadeStart < 0.0f) fadeStart = 0.0f;
        if (age > fadeStart) alpha = (sg_modWaypointsLifetime - age) / 2.0f;
        if (alpha <= 0) return;
        
        
        REAL pulse = 0.7f + 0.3f * sin(age * 3.0f);
        
        
        {
            ModelMatrix();
            glPushMatrix();
            glTranslatef(pos.x, pos.y, 0);
            
            glDisable(GL_TEXTURE_2D);
            
            REAL diamondSize = 0.4f * pulse;
            REAL diamondHeight = 3.0f + 0.3f * sin(age * 2.0f); 
            
            
            glColor4f(color_.r, color_.g, color_.b, alpha * 0.8f);
            glBegin(GL_TRIANGLES);
            
            glVertex3f(0, 0, diamondHeight + diamondSize);
            glVertex3f(diamondSize, 0, diamondHeight);
            glVertex3f(0, diamondSize, diamondHeight);
            
            glVertex3f(0, 0, diamondHeight + diamondSize);
            glVertex3f(0, diamondSize, diamondHeight);
            glVertex3f(-diamondSize, 0, diamondHeight);
            
            glVertex3f(0, 0, diamondHeight + diamondSize);
            glVertex3f(-diamondSize, 0, diamondHeight);
            glVertex3f(0, -diamondSize, diamondHeight);
            
            glVertex3f(0, 0, diamondHeight + diamondSize);
            glVertex3f(0, -diamondSize, diamondHeight);
            glVertex3f(diamondSize, 0, diamondHeight);
            
            
            glVertex3f(0, 0, diamondHeight - diamondSize);
            glVertex3f(0, diamondSize, diamondHeight);
            glVertex3f(diamondSize, 0, diamondHeight);
            
            glVertex3f(0, 0, diamondHeight - diamondSize);
            glVertex3f(-diamondSize, 0, diamondHeight);
            glVertex3f(0, diamondSize, diamondHeight);
            
            glVertex3f(0, 0, diamondHeight - diamondSize);
            glVertex3f(0, -diamondSize, diamondHeight);
            glVertex3f(-diamondSize, 0, diamondHeight);
            
            glVertex3f(0, 0, diamondHeight - diamondSize);
            glVertex3f(diamondSize, 0, diamondHeight);
            glVertex3f(0, -diamondSize, diamondHeight);
            glEnd();
            
            
            glColor4f(color_.r, color_.g, color_.b, alpha * 0.4f);
            glBegin(GL_LINES);
            glVertex3f(0, 0, 0);
            glVertex3f(0, 0, diamondHeight - diamondSize);
            glEnd();
            
            glPopMatrix();
        }
        
        
        {
            float modelviewMatrix[16], projectionMatrix[16];
            float x, y, z, w;
            float xp, yp, wp;

            glPushMatrix();
            glTranslatef(pos.x, pos.y, 4.0f);
            glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);
            glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
            glPopMatrix();

            x = modelviewMatrix[12]; y = modelviewMatrix[13]; 
            z = modelviewMatrix[14]; w = modelviewMatrix[15];

            xp = projectionMatrix[0]*x + projectionMatrix[4]*y + projectionMatrix[8]*z + projectionMatrix[12]*w;
            yp = projectionMatrix[1]*x + projectionMatrix[5]*y + projectionMatrix[9]*z + projectionMatrix[13]*w;
            wp = projectionMatrix[3]*x + projectionMatrix[7]*y + projectionMatrix[11]*z + projectionMatrix[15]*w;

            if (wp <= 0) return;

            xp /= wp; yp /= wp;
            if (xp <= -1 || xp >= 1 || yp <= -1 || yp >= 1) return;

            ModelMatrix();
            glPushMatrix();
            glLoadIdentity();

            ProjMatrix();
            glPushMatrix();
            glLoadIdentity();

            glTranslatef(xp, yp, 0.);
            
            
            {
                float bar_w = 0.12f;
                float bar_h = 0.025f;
                float bar_y = -0.01f;
                
                glDisable(GL_TEXTURE_2D);
                
                
                glColor4f(0, 0, 0, alpha * 0.6f);
                glBegin(GL_QUADS);
                glVertex2f(-bar_w/2, bar_y);
                glVertex2f(bar_w/2, bar_y);
                glVertex2f(bar_w/2, bar_y + bar_h);
                glVertex2f(-bar_w/2, bar_y + bar_h);
                glEnd();
                
                
                REAL maxRubber = sg_rubberCycle;
                REAL ratio = (maxRubber > 0.001f) ? (amount_ / maxRubber) : 0.0f;
                if (ratio > 1.0f) ratio = 1.0f;
                
                glColor4f(color_.r * pulse, color_.g * pulse, color_.b, alpha * 0.9f);
                glBegin(GL_QUADS);
                glVertex2f(-bar_w/2 + 0.003f, bar_y + 0.003f);
                glVertex2f(-bar_w/2 + 0.003f + (bar_w - 0.006f) * ratio, bar_y + 0.003f);
                glVertex2f(-bar_w/2 + 0.003f + (bar_w - 0.006f) * ratio, bar_y + bar_h - 0.003f);
                glVertex2f(-bar_w/2 + 0.003f, bar_y + bar_h - 0.003f);
                glEnd();
                
                
                glColor4f(1.0f, 1.0f, 1.0f, alpha * 0.7f);
                glBegin(GL_LINE_LOOP);
                glVertex2f(-bar_w/2, bar_y);
                glVertex2f(bar_w/2, bar_y);
                glVertex2f(bar_w/2, bar_y + bar_h);
                glVertex2f(-bar_w/2, bar_y + bar_h);
                glEnd();
                
                glEnable(GL_TEXTURE_2D);
            }
            
            
            rTextField::SetBlendColor(tColor(1,1,1,alpha));
            char rubTxt[16];
            snprintf(rubTxt, sizeof(rubTxt), "%.1f", amount_);
            
            tColoredString finalTxt;
            finalTxt << tColoredString::ColorString(color_.r, color_.g, color_.b) << rubTxt;
            
            DisplayText(0, 0.03f, 0.008f, 0.02f, finalTxt, 0, 0);
            rTextField::SetDefaultColor(tColor(1,1,1));

            ProjMatrix();
            glPopMatrix();
            ModelMatrix();
            glPopMatrix();
        }
    }
#endif
    
private:
    REAL amount_;
    gRealColor color_;
    REAL creationTime_;
};


bool sg_gnuplotDebug = false;

#define GNUPLOT_DEBUG


#ifdef GNUPLOT_DEBUG
static tSettingItem<bool> sg_("DEBUG_GNUPLOT",sg_gnuplotDebug);
#endif

static REAL sg_minDropInterval=0.05;
static tSettingItem< REAL > sg_minDropIntervalConf( "CYCLE_MIN_WALLDROP_INTERVAL", sg_minDropInterval );

#ifdef DEDICATED
static bool sg_predictWalls=true;
static tSettingItem< bool > sg_predictWallsConf( "PREDICT_WALLS", sg_predictWalls );
#endif



static nNOInitialisator<gCycle> cycle_init(320,"cycle");







bool headlights=1;
extern bool cycleprograminited;

static float sg_cycleSyncSmoothTime = .1f;
static tSettingItem<float> conf_smoothTime ("CYCLE_SMOOTH_TIME", sg_cycleSyncSmoothTime);

static float sg_cycleSyncSmoothMinSpeed = .2f;
static tSettingItem<float> conf_smoothMinSpeed ("CYCLE_SMOOTH_MIN_SPEED", sg_cycleSyncSmoothMinSpeed);

static float sg_cycleSyncSmoothThreshold = .2f;
static tSettingItem<float> conf_smoothThreshold ("CYCLE_SMOOTH_THRESHOLD", sg_cycleSyncSmoothThreshold);

static REAL sg_enemyChatbotTimePenalty = 30.0f;   
static tSettingItem<REAL> sg_enemyChatbotTimePenaltyConf( "ENEMY_CHATBOT_PENALTY", sg_enemyChatbotTimePenalty );
extern REAL sg_suicideTimeout;

static inline void clamp(REAL &c, REAL min, REAL max){
    tASSERT(min < max);

    if (!std::isfinite(c))
        c = 0;

    if (c<min)
        c = min;

    if (c>max)
        c = max;
}


REAL		gCycle::wallsStayUpDelay=8.0f;	

REAL		gCycle::wallsLength=-1.0f;		
REAL		gCycle::explosionRadius=4.0f;	

static		nSettingItem<REAL> *c_pwsud = NULL, *c_pwl = NULL, *c_per = NULL;

void gCycle::PrivateSettings()
{
    static nSettingItem<REAL> c_wsud("CYCLE_WALLS_STAY_UP_DELAY",wallsStayUpDelay);
    static nSettingItem<REAL> c_wl("CYCLE_WALLS_LENGTH",wallsLength);
    static nSettingItem<REAL> c_er("CYCLE_EXPLOSION_RADIUS",explosionRadius);

    c_pwsud=&c_wsud;
    c_pwl  =&c_wl;
    c_per  =&c_er;
}


static REAL sg_speedCycleSound=15;
static nSettingItem<REAL> c_ss("CYCLE_SOUND_SPEED",
                               sg_speedCycleSound);


static REAL sg_cycleWallTime=0.0;
static nSettingItemWatched<REAL>
sg_cycleWallTimeConf("CYCLE_WALL_TIME",
                     sg_cycleWallTime,
                     nConfItemVersionWatcher::Group_Bumpy,
                     14);


static REAL sg_cycleInvulnerableTime=0.0;
static nSettingItemWatched<REAL>
sg_cycleInvulnerableTimeConf("CYCLE_INVULNERABLE_TIME",
                             sg_cycleInvulnerableTime,
                             nConfItemVersionWatcher::Group_Bumpy,
                             12);


static bool sg_cycleFirstSpawnProtection=false;
static nSettingItemWatched<bool>
sg_cycleFirstSpawnProtectionConf("CYCLE_FIRST_SPAWN_PROTECTION",
                                 sg_cycleFirstSpawnProtection,
                                 nConfItemVersionWatcher::Group_Bumpy,
                                 12);


static REAL sg_syncIntervalEnemy=1;
static tSettingItem<REAL> c_sie( "CYCLE_SYNC_INTERVAL_ENEMY",
                                 sg_syncIntervalEnemy );

static REAL sg_syncIntervalSelf=.1;
static tSettingItem<REAL> c_sis( "CYCLE_SYNC_INTERVAL_SELF",
                                 sg_syncIntervalSelf );


static bool sg_avoidBadOldClientSync=true;
static tSettingItem<bool> c_sbs( "CYCLE_AVOID_OLDCLIENT_BAD_SYNC",
                                 sg_avoidBadOldClientSync );


static REAL sg_syncFF=10;
static tSettingItem<REAL> c_sff( "CYCLE_SYNC_FF",
                                 sg_syncFF );

static int sg_syncFFSteps=1;
static tSettingItem<int> c_sffs( "CYCLE_SYNC_FF_STEPS",
                                 sg_syncFFSteps );


static nVersionFeature sg_NoLocalTunnelOnSync( 4 );

static REAL sg_GetSyncIntervalSelf( gCycle* cycle )
{
    if ( sg_NoLocalTunnelOnSync.Supported( cycle->Owner() ) )
        return sg_syncIntervalSelf;
    else
        return sg_syncIntervalEnemy * 10;
}





static int score_hole=0;
static tSettingItem<int> s_h("SCORE_HOLE",score_hole);

static int score_survive=0;
static tSettingItem<int> s_sur("SCORE_SURVIVE",score_survive);

static int score_die=-2;
static tSettingItem<int> s_d("SCORE_DIE",score_die);

static int score_kill=3;
static tSettingItem<int> s_k("SCORE_KILL",score_kill);

static int score_suicide=-4;
static tSettingItem<int> s_s("SCORE_SUICIDE",score_suicide);



uActionPlayer gCycle::s_brake("CYCLE_BRAKE", -9);
static uActionPlayer s_brakeToggle("CYCLE_BRAKE_TOGGLE", -9);
static uActionTooltip sg_brakeTooltip( gCycle::s_brake, 1, &ePlayer::VetoActiveTooltip );

uActionPlayer gCycle::se_perfectTurnLeft("CYCLE_PERFECT_LEFT", -9);
uActionPlayer gCycle::se_perfectTurnRight("CYCLE_PERFECT_RIGHT", -9);
uActionPlayer gCycle::se_autoEscapeLeft("CYCLE_AUTOESCAPE_LEFT", -9);
uActionPlayer gCycle::se_autoEscapeRight("CYCLE_AUTOESCAPE_RIGHT", -9);


REAL sg_autoEscapeRubberMargin = 0.0f;


bool sg_autoEscapePingComp = false;



REAL sg_perfectTurnCalibration = 0.0f;


static eWavData cycle_run("moviesounds/engine.wav","sound/cyclrun.wav");
static eWavData turn_wav("moviesounds/cycturn.wav","sound/expl.wav");
static eWavData scrap("sound/expl.wav");



class gTextureCycle: public rSurfaceTexture
{
    gRealColor color_; 
    bool wheel; 
public:
    gTextureCycle(rSurface const & surface, const gRealColor& color,bool repx=0,bool repy=0,bool wheel=false);

    virtual void ProcessImage(SDL_Surface *im);

    virtual void OnSelect(bool enforce);
    using rSurfaceTexture::OnSelect;
};

gTextureCycle::gTextureCycle(rSurface const & surface, const gRealColor& color,bool repx,bool repy,bool w)
        :rSurfaceTexture(rTextureGroups::TEX_OBJ,surface,repx,repy),
        color_(color),wheel(w)
{
    Select();
}

void gTextureCycle::ProcessImage(SDL_Surface *im)
{
#ifndef DEDICATED
    
    tVERIFY(SDL_BYTESPERPIXEL(im->format) == 4);
    GLubyte R=int(color_.r*255);
    GLubyte G=int(color_.g*255);
    GLubyte B=int(color_.b*255);

    GLubyte *pixels =reinterpret_cast<GLubyte *>(im->pixels);

    for(int i=im->w*im->h-1;i>=0;i--){
        GLubyte alpha=pixels[4*i+3];
        pixels[4*i  ] = (alpha * pixels[4*i  ] + (255-alpha)*R) >> 8;
        pixels[4*i+1] = (alpha * pixels[4*i+1] + (255-alpha)*G) >> 8;
        pixels[4*i+2] = (alpha * pixels[4*i+2] + (255-alpha)*B) >> 8;
        pixels[4*i+3] = 255;
    }
#endif
}

void gTextureCycle::OnSelect(bool enforce){
#ifndef DEDICATED
    rISurfaceTexture::OnSelect(enforce);

    if(rTextureGroups::TextureMode[rTextureGroups::TEX_OBJ]<0){
        REAL R=color_.r,G=color_.g,B=color_.b;
        if(wheel){
            R*=.7;
            G*=.7;
            B*=.7;
        }
        glColor3f(R,G,B);
        GLfloat color[4]={R,G,B,1};

        glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,color);
        glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,color);
    }
#endif
}


extern void sg_RubberValues( ePlayerNetID const * player, REAL speed, REAL & max, REAL & effectiveness );
extern REAL sg_brakeCycle;
extern REAL sg_cycleBrakeDeplete;


#ifdef DEBUG
#ifndef DEDICATED
#define DEBUGCHATBOT
#endif
#endif

#ifdef DEBUGCHATBOT
typedef tSettingItem<REAL> gChatBotSetting;
typedef tSettingItem<bool> gChatBotSwitch;
#else
typedef nSettingItem<REAL> gChatBotSetting;
typedef nSettingItem<bool> gChatBotSwitch;
#endif

static bool sg_chatBotAlwaysActive = false;
static gChatBotSwitch sg_chatBotAlwaysActiveConf( "CHATBOT_ALWAYS_ACTIVE", sg_chatBotAlwaysActive );

static REAL sg_chatBotNewWallBlindness = .3;
static gChatBotSetting sg_chatBotNewWallBlindnessConf( "CHATBOT_NEW_WALL_BLINDNESS",
        sg_chatBotNewWallBlindness );

static REAL sg_chatBotMinTimestep = .3;
static gChatBotSetting sg_chatBotMinTimestepConf( "CHATBOT_MIN_TIMESTEP",
        sg_chatBotMinTimestep );

static REAL sg_chatBotDelay = .5;
static gChatBotSetting sg_chatBotDelayConf( "CHATBOT_DELAY",
        sg_chatBotDelay );

static REAL sg_chatBotRange = 1;
static gChatBotSetting sg_chatBotRangeConf( "CHATBOT_RANGE",
        sg_chatBotRange );

static REAL sg_chatBotDecay = .02;
static gChatBotSetting sg_chatBotDecayConf( "CHATBOT_DECAY",
        sg_chatBotDecay );

class gCycleChatBot
{
    gCycleChatBot();
public:
class Sensor: public gSensor
    {
    public:
        Sensor(gCycle *o,const eCoord &start,const eCoord &d)
                : gSensor(o,start,d)
                , hitOwner_( 0 )
                , hitTime_ ( 0 )
                , hitDistance_( o->MaxWallsLength() )
                , lrSuggestion_( 0 )
                , windingNumber_( 0 )
        {
            if ( hitDistance_ <= 0 )
                hitDistance_ = o->GetDistance();
        }

        

        virtual void PassEdge(const eWall *ww,REAL time,REAL a,int r)
        {
            try{
                gSensor::PassEdge(ww,time,a,r);
            }
            catch( eSensorFinished & e )
            {
                if ( DoExtraDetectionStuff() )
                    throw;
            }
        }

        bool DoExtraDetectionStuff()
        {
            
            lrSuggestion_ = -lr;

            switch ( type )
            {
            case gSENSOR_NONE:
            case gSENSOR_RIM:
                lrSuggestion_ = 0;
                return true;
            default:
                
                
            case gSENSOR_SELF:
                {
                    
                    if ( !ehit )
                        return true;
                    eWall * wall = ehit->GetWall();
                    if ( !wall )
                        return true;
                    gPlayerWall * playerWall = dynamic_cast< gPlayerWall * >( wall );
                    if ( !playerWall )
                        return true;
                    hitOwner_ = playerWall->Cycle();
                    if ( !hitOwner_ )
                        return true;

                    

                    REAL wallAlpha = playerWall->Edge()->Ratio( before_hit );
                    
                    if ( wallAlpha < 0 )
                        wallAlpha = 0;
                    if ( wallAlpha > 1 )
                        wallAlpha = 1;
                    hitDistance_   = hitOwner_->GetDistance() - playerWall->Pos( wallAlpha );
                    hitTime_       = playerWall->Time( wallAlpha );
                    windingNumber_ = playerWall->WindingNumber();

                    
                    if ( hitTime_ > hitOwner_->LastTime() - sg_chatBotNewWallBlindness && hitOwner_ != owned )
                    {
                        ehit = NULL;
                        hit = 1E+40;
                        return false;
                    }

                    

                    

                    
                }
            }

            return true;
        }

        
        REAL HitWallExtends( eCoord const & dir, eCoord const & origin )
        {
            if ( !ehit || !ehit->Other() )
            {
                return 1E+30;
            }

            REAL ret = -1E+30;
            eCoord ends[2] = { *ehit->Point(), *ehit->Other()->Point() };
            for ( int i = 1; i>=0; --i )
            {
                REAL newRet = eCoord::F( dir, ends[i]-origin );
                if ( newRet > ret )
                    ret = newRet;
            }

            return ret;
        }

        gCycle * hitOwner_;     
        REAL     hitTime_;      
        REAL     hitDistance_;  
        short    lrSuggestion_; 
        int      windingNumber_; 
    };

    gCycleChatBot( gCycle * owner )
            : nextChatAI_( 0 )
            , timeOnChatAI_( 0 )
            , lastTurn_( 0 )
            , nextTurn_ ( 0 )
            , turnedRecently_ ( 0 )
            , owner_ ( owner )
    {
#ifdef RLBOT
        rlDir = 1;
        rlLastTime = -100;
#endif
    }

    
    class WallHug
    {
    public:
        gCycle const * owner_;  
        REAL lastTimeSeen_;    

        WallHug()
                : owner_ ( NULL )
                , lastTimeSeen_ ( 0 )
        {
        }
    };

    
    void FindHugReplacement( Sensor const & sensor )
    {
        gCycle const * owner = sensor.hitOwner_;
        if (!owner)
            return;

        
        if ( !hugReplacement_.owner_ && sensor.type != gSENSOR_SELF &&
                owner != hugLeft_.owner_ &&
                owner != hugRight_.owner_ )
        {
            hugReplacement_.owner_ = sensor.hitOwner_;
            hugReplacement_.lastTimeSeen_ = nextChatAI_;
        }

        
        if ( owner == hugLeft_.owner_ )
            hugLeft_.lastTimeSeen_ = nextChatAI_;
        if ( owner == hugRight_.owner_ )
            hugRight_.lastTimeSeen_ = nextChatAI_;
    }

    
    
    REAL Distance( Sensor const & a, Sensor const & b )
    {
        
        if ( a.Direction() * b.Direction() < 0 )
            return Distance( b, a );

        bool self = a.type == gSENSOR_SELF || b.type == gSENSOR_SELF;
        bool rim  = a.type == gSENSOR_RIM || b.type == gSENSOR_RIM;

        
        REAL selfHatred = 1;
        if ( a.type == gSENSOR_SELF )
        {
            selfHatred *= .5;
            if ( a.lr > 0 )
            {
                selfHatred *= .5;
                if ( b.type == gSENSOR_RIM )
                    selfHatred *= .25;
            }
        }
        if ( b.type == gSENSOR_SELF )
        {
            selfHatred *= .5;
            if ( b.lr < 0 )
            {
                selfHatred *= .5;
                if ( a.type == gSENSOR_RIM )
                    selfHatred *= .25;
            }
        }

        
        REAL bigDistance = owner_->MaxWallsLength();
        if ( bigDistance <= 0 )
            bigDistance = owner_->GetDistance();

        if ( a.hitOwner_ != b.hitOwner_ )
        {
            
            REAL ret =
                a.hitDistance_ + b.hitDistance_;

            if ( rim )
            {
                ret = bigDistance * .001 + ret * .01 + ( a.before_hit - b.before_hit).Norm();

                
                if ( !self )
                    ret = bigDistance * 2;
            }

            
            
            ret *= 16;

            
            if ( a.type == gSENSOR_NONE || b.type == gSENSOR_NONE )
                ret *= 2;

            return ret * selfHatred;
        }
        else if ( rim )
        {
            
            return ( a.before_hit - b.before_hit).Norm() * selfHatred;
        }
        else if ( a.type == gSENSOR_NONE && b.type == gSENSOR_NONE )
        {
            
            return owner_->GetDistance() * 256;
        }
        else if ( a.lr != b.lr )
        {
            
            return ( fabsf( a.hitDistance_ - b.hitDistance_ ) + .25 * bigDistance ) * selfHatred;
        }
        
        else
        {
            
            return fabsf( a.hitDistance_ - b.hitDistance_ ) * selfHatred;
        }

        
        return ( a.before_hit - b.before_hit).Norm() * selfHatred;
    }

    static gCycleChatBot & Get( gCycle * cycle )
    {
        tASSERT( cycle );

        
        if ( cycle->chatBot_.get() == 0 )
            cycle->chatBot_.reset( new gCycleChatBot( cycle ) );

        return *cycle->chatBot_;
    }

    bool CanMakeTurn( uActionPlayer * action )
    {
        return owner_->CanMakeTurn( ( action == &gCycle::se_turnRight ) ? 1 : -1 );
    }

#ifdef RLBOT
    int rlDir;
    REAL rlLastTime;
#endif

    
    void Activate( REAL currentTime )
    {
#ifdef RLBOT
        
        {
            if (!owner_->Alive() || !owner_->Vulnerable() )
            {
                return;
            }
            if( fabs( rlLastTime - currentTime) > 1 )
            {
                owner_->Act( &gCycle::se_turnRight, 1 );
                rlDir = -1;
            }
            else  if ( rlDir > 0 )
            {
                if( CanMakeTurn( &gCycle::se_turnRight ) )
                {
                    owner_->Act( &gCycle::se_turnRight, 1 );
                    owner_->Act( &gCycle::se_turnRight, 1 );
                    owner_->Act( &gCycle::se_turnRight, 1 );
                    rlDir = -1;
                }
            }
            else
            {
                if( CanMakeTurn( &gCycle::se_turnLeft ) )
                {
                    owner_->Act( &gCycle::se_turnLeft, 1 );
                    owner_->Act( &gCycle::se_turnLeft, 1 );
                    owner_->Act( &gCycle::se_turnLeft, 1 );
                    rlDir = 1;
                }
            }
            rlLastTime = currentTime;
            return;
        }
#endif

        
        if ( currentTime < nextChatAI_ )
            return;

        REAL lookahead = sg_chatBotRange;  
        REAL minstep   = sg_chatBotMinTimestep; 
        REAL maxstep   = sg_chatBotMinTimestep * 4 * ( 1 + .1 * tReproducibleRandomizer::GetInstance().Get() );  

        
        if ( nextChatAI_ <= EPS )
        {
            nextChatAI_ = sg_chatBotDelay + currentTime;
            return;
        }

        timeOnChatAI_ += currentTime - nextChatAI_;

        
        REAL speed = owner_->Speed();
        eCoord dir = owner_->Direction();
        eCoord pos = owner_->Position();

        
        if ( sn_GetNetState() != nSTANDALONE )
        {
            REAL qualityFactor = ( timeOnChatAI_ * sg_chatBotDecay );
            if ( qualityFactor > 1 )
            {
                minstep *= qualityFactor;
                
            }
        }

        REAL range= speed * lookahead;
        eCoord scanDir = dir * range;

        REAL frontFactor = .5;

        Sensor front(owner_,pos,scanDir);
        front.detect(frontFactor);
        owner_->enemyInfluence.AddSensor( front, sg_enemyChatbotTimePenalty, owner_ );

        REAL minMoveOn = 0, maxMoveOn = 0, moveOn = 0;

        
        REAL rubberGranted, rubberEffectiveness;
        sg_RubberValues( owner_->player, speed, rubberGranted, rubberEffectiveness );
        REAL rubberTime = ( rubberGranted - owner_->GetRubber() )*rubberEffectiveness/speed;
        REAL rubberRatio = owner_->GetRubber()/rubberGranted;

        if ( front.ehit )
        {
            turnedRecently_ = false;

            
            tJUST_CONTROLLED_PTR< gNetPlayerWall > lastWall = owner_->lastWall;
            owner_->lastWall = NULL;

            REAL narrowFront = 1;

            
            Sensor forwardLeft ( owner_, pos, scanDir.Turn(+1,+1 ) );
            Sensor backwardLeft( owner_, pos, scanDir.Turn(-1,+narrowFront) );
            forwardLeft.detect(1);
            backwardLeft.detect(1);
            Sensor forwardRight ( owner_, pos, scanDir.Turn(+1,-1 ) );
            Sensor backwardRight( owner_, pos, scanDir.Turn(-1,-narrowFront) );
            forwardRight.detect(1);
            backwardRight.detect(1);

            
            if ( hugReplacement_.owner_ && !hugLeft_.owner_ && !hugRight_.owner_ )
            {
                
                int lr = 0;
                if ( backwardLeft.hitOwner_ == hugReplacement_.owner_ )
                    lr--;
                if ( forwardLeft.hitOwner_ == hugReplacement_.owner_ )
                    lr--;
                if ( backwardRight.hitOwner_ == hugReplacement_.owner_ )
                    lr++;
                if ( forwardRight.hitOwner_ == hugReplacement_.owner_ )
                    lr++;

                if ( lr > 0 )
                    hugRight_ = hugReplacement_;
                if ( lr < 0 )
                    hugLeft_ = hugReplacement_;

                hugReplacement_.owner_ = 0;
            }

            if ( hugReplacement_.owner_ )
            {
                if( hugLeft_.lastTimeSeen_ < hugRight_.lastTimeSeen_ )
                {
                    if ( hugReplacement_.lastTimeSeen_ > hugLeft_.lastTimeSeen_ )
                        hugLeft_ = hugReplacement_;
                }
                else
                {
                    if ( hugReplacement_.lastTimeSeen_ > hugRight_.lastTimeSeen_ )
                        hugRight_ = hugReplacement_;
                }
                hugReplacement_.owner_ = 0;
            }

            FindHugReplacement( front );
            FindHugReplacement( forwardLeft );
            FindHugReplacement( forwardRight );
            FindHugReplacement( backwardLeft );
            FindHugReplacement( backwardRight );

            
            REAL frontOpen = Distance ( forwardLeft, forwardRight );
            REAL leftOpen  = Distance ( forwardLeft, backwardLeft );
            REAL rightOpen = Distance ( forwardRight, backwardRight );
            REAL rearOpen = Distance ( backwardLeft, backwardRight );

            Sensor self( owner_, pos, scanDir.Turn(-1, 0) );
            
            self.before_hit = pos;
            self.windingNumber_ = owner_->windingNumber_;
            self.type = gSENSOR_SELF;
            self.hitDistance_ = 0;
            self.hitOwner_ = owner_;
            self.hitTime_ = currentTime;
            self.lr = -1;
            REAL rearLeftOpen = Distance( backwardLeft, self );
            self.lr = 1;
            REAL rearRightOpen = Distance( backwardRight, self );

            

            
            if ( forwardRight.type == gSENSOR_SELF &&
                    forwardLeft.type == gSENSOR_RIM &&
                    backwardRight.type == gSENSOR_SELF &&
                    backwardLeft.type == gSENSOR_RIM &&
                    
                    forwardRight.lr == -1 &&
                    backwardRight.lr == -1 )
            {
                turnedRecently_ = true;
                if ( rightOpen > speed * ( owner_->GetTurnDelay() - rubberTime * .8 ) )
                {
                    owner_->Act( &gCycle::se_turnRight, 1 );
                    owner_->Act( &gCycle::se_turnRight, 1 );
                }
                else
                {
                    owner_->Act( &gCycle::se_turnLeft, 1 );
                    owner_->Act( &gCycle::se_turnLeft, 1 );
                }
            }

            if ( forwardLeft.type == gSENSOR_SELF &&
                    forwardRight.type == gSENSOR_RIM &&
                    backwardLeft.type == gSENSOR_SELF &&
                    backwardRight.type == gSENSOR_RIM &&
                    
                    forwardLeft.lr == 1 &&
                    backwardLeft.lr == 1 )
            {
                turnedRecently_ = true;
                if ( leftOpen > speed * ( owner_->GetTurnDelay() - rubberTime * .8 ) )
                {
                    owner_->Act( &gCycle::se_turnLeft, 1 );
                    owner_->Act( &gCycle::se_turnLeft, 1 );
                }
                else
                {
                    owner_->Act( &gCycle::se_turnRight, 1 );
                    owner_->Act( &gCycle::se_turnRight, 1 );
                }
            }

            
            uActionPlayer * bestAction = ( leftOpen > rightOpen ) ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
            int             bestDir      = ( leftOpen > rightOpen ) ? 1 : -1;
            REAL            bestOpen     = ( leftOpen > rightOpen ) ? leftOpen : rightOpen;
            Sensor &        bestForward  = ( leftOpen > rightOpen ) ? forwardLeft : forwardRight;
            Sensor &        bestBackward = ( leftOpen > rightOpen ) ? backwardLeft : backwardRight;

            Sensor direct ( owner_, pos, scanDir.Turn( 0, bestDir) );
            direct.detect( 1 );

            
            owner_->lastWall = lastWall;

            
            
            REAL forwardHalf  = Distance ( direct, bestForward );
            REAL backwardHalf = Distance ( direct, bestBackward );

            REAL forwardOverhang  = bestForward.HitWallExtends( bestForward.Direction(), pos );
            REAL backwardOverhang  = bestBackward.HitWallExtends( bestForward.Direction(), pos );

            
            minMoveOn = bestBackward.HitWallExtends( dir, pos );

            
            REAL minMoveOnOther = direct.HitWallExtends( dir, pos );

            
            maxMoveOn      = bestForward.HitWallExtends( dir, pos );
            REAL maxMoveOnOther = front.HitWallExtends( dir, pos );
            if ( maxMoveOn > maxMoveOnOther )
                maxMoveOn = maxMoveOnOther;

            if ( maxMoveOn > minMoveOnOther && forwardHalf > backwardHalf && direct.hitOwner_ == bestBackward.hitOwner_ )
            {
                backwardOverhang  = direct.HitWallExtends( bestForward.Direction(), pos );
                minMoveOn = minMoveOnOther;
            }

            
            moveOn = .5 * ( minMoveOn * ( 1 + rubberRatio ) + maxMoveOn * ( 1 - rubberRatio ) );

            
            bool brake = sg_brakeCycle > 0 &&
                         front.hit * lookahead * sg_cycleBrakeDeplete < owner_->GetBrakingReservoir() &&
                         sg_brakeCycle * front.hit * lookahead < 2 * speed * owner_->GetBrakingReservoir() &&
                         ( maxMoveOn - minMoveOn ) > 0 &&
                         owner_->GetBrakingReservoir() * ( maxMoveOn - minMoveOn ) < speed * owner_->GetTurnDelay();
            if ( frontOpen < bestOpen &&
                    ( forwardOverhang <= backwardOverhang || ( minMoveOn < 0 && moveOn < minstep * speed ) ) )
            {
                
                

                
                
                
                turnedRecently_ = true;

                minMoveOn = maxMoveOn = moveOn = 0;

                
                {
                    if ( !CanMakeTurn( bestAction ) )
                    {
                        nextChatAI_ = currentTime;
                        return;
                    }

                    owner_->Act( bestAction, 1 );
                }

                brake = false;
            }
            else
            {
                
                REAL bestSoFar = frontOpen > bestOpen ? frontOpen : bestOpen;
                bestSoFar *= ( 10 * ( 1 - rubberRatio ) + 1 );

                if ( rearOpen > bestSoFar && ( rearLeftOpen > bestSoFar || rearRightOpen > bestSoFar ) )
                {
                    brake = false;
                    turnedRecently_ = true;

                    bool goLeft = rearLeftOpen > rearRightOpen;

                    
                    uActionPlayer * bestAction = goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
                    uActionPlayer * otherAction = !goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
                    Sensor &        bestForward  = goLeft ? forwardLeft : forwardRight;
                    Sensor &        bestBackward  = goLeft ? backwardLeft : backwardRight;
                    Sensor &        otherForward  = !goLeft ? forwardLeft : forwardRight;
                    Sensor &        otherBackward  = !goLeft ? backwardLeft : backwardRight;

                    
                    REAL bestHit = bestForward.hit > bestBackward.hit ? bestBackward.hit : bestForward.hit;
                    REAL otherHit = otherForward.hit > otherBackward.hit ? otherBackward.hit : otherForward.hit;

                    bool wait = false;

                    if ( !CanMakeTurn( bestAction ) )
                    {
                        nextChatAI_ = currentTime;
                        return;
                    }

                    
                    if ( bestHit * lookahead < owner_->GetTurnDelay() + rubberTime )
                    {
                        if ( otherHit < bestForward.hit * 2 && front.hit * lookahead > owner_->GetTurnDelay() * 2 )
                        {
                            
                            wait = true;
                        }
                        else
                        {
                            if ( !CanMakeTurn( otherAction ) )
                            {
                                nextChatAI_ = currentTime;
                                return;
                            }

                            owner_->Act( otherAction, 1 );

                            
                            if ( maxMoveOn < speed * owner_->GetTurnDelay() )
                            {
                                
                                owner_->Act( otherAction, 1 );
                                wait = true;
                            }
                        }
                    }

                    if ( !wait )
                    {
                        owner_->Act( bestAction, 1 );
                        owner_->Act( bestAction, 1 );
                    }

                    minMoveOn = maxMoveOn = moveOn = 0;
                }
            }

            
            owner_->Act( &gCycle::s_brake, brake ? 1 : -1 );

            
            if ( hugLeft_.owner_ == backwardRight.hitOwner_ ||
                    hugRight_.owner_ == backwardLeft.hitOwner_ )
            {
                WallHug swap = hugRight_;
                hugRight_ = hugLeft_;
                hugLeft_ = swap;
            }
        }

        

        
        

        
        
        

        
        
        
        

        
        
        
        
        
        

        REAL space = moveOn;
        REAL minTime = space/speed;

        if ( turnedRecently_ )
            minTime = owner_->GetTurnDelay();

        if ( minTime < minstep )
            minTime = minstep;
        if ( minTime > maxstep + minstep * 1.5 )
        {
            minTime = maxstep;
        }
        

        nextChatAI_ = currentTime + minTime;
        timeOnChatAI_ += minTime;
    }

    REAL nextChatAI_;        
private:
    REAL timeOnChatAI_;      
    short lastTurn_;         
    REAL nextTurn_;          
    bool turnedRecently_;    
    gCycle * owner_;         

    WallHug hugLeft_;              
    WallHug hugRight_;             
    WallHug hugReplacement_;       
};



static void sg_ArchiveCoord( eCoord & coord, int level )
{
    static char const * section = "_COORD";
    tRecorderSync< eCoord >::Archive( section, level, coord );
}

static void sg_ArchiveReal( REAL & real, int level )
{
    static char const * section = "_REAL";
    tRecorderSync< REAL >::Archive( section, level, real );
}









gDestination::gDestination(const gCycleMovement &c)
        :chatting(false)
        ,turns(0)
        ,hasBeenUsed(false)
        ,messageID(1)
        ,missable(true)
        ,next(NULL)
        ,list(NULL)
{
    CopyFrom( c );
}

gDestination::gDestination(const gCycle &c)
        :chatting(false)
        ,turns(0)
        ,hasBeenUsed(false)
        ,messageID(1)
        ,missable(true)
        ,next(NULL)
        ,list(NULL)
{
    CopyFrom( c );
}


gDestination::gDestination(nMessage &m, unsigned short & cycle_id )
        :gameTime(0),distance(0),speed(0),
        hasBeenUsed(false),
        messageID(1),
        missable(true),
next(NULL),list(NULL){
    m >> position;
    m >> direction;
    m >> distance;

    unsigned short flags;
    m >> flags;
    braking  = flags & 0x01;
    chatting = flags & 0x02;

    messageID = m.MessageID();

    turns = 0;

    m.Read( cycle_id );

    if ( !m.End() )
        m >> gameTime;
    else
        gameTime = -1000;

    if ( !m.End() )
    {
        m.Read( turns );
    }
}

void gDestination::CopyFrom(const gCycleMovement &other)
{
    position 	= other.Position();
    direction 	= other.Direction();
    gameTime 	= other.LastTime();
    distance 	= other.GetDistance();
    speed 		= other.Speed();
    braking 	= other.GetBraking();
    turns 		= other.GetTurns();

#ifdef DEBUG
    if (!std::isfinite(gameTime) || !std::isfinite(speed) || !std::isfinite(distance))
        st_Breakpoint();
#endif
    if ( other.Owner() && other.Player() )
        chatting = other.Player()->IsChatting();

    
    if ( other.RubberDepleteTime() > 0 )
    {
        gameTime = other.RubberDepleteTime();
    }
}

void gDestination::CopyFrom(const gCycle &other)
{
    CopyFrom( static_cast<const gCycleMovement&>(other) );

    
    distance 	+= other.correctDistanceSmooth;
}












int gDestination::CompareWith( const gDestination & other ) const
{
    

    
    if ( messageID > 1 && other.messageID > 1 )
    {
        short messageIDDifference = messageID - other.messageID;
        if ( messageIDDifference < 0 )
            return -1;
        if ( messageIDDifference > 0 )
            return 1;
    }

    
    if ( distance < other.distance )
        return -1;
    else if ( distance > other.distance )
        return 1;

    
    return 0;
}

class gFloatCompressor
{
public:
    gFloatCompressor( REAL min, REAL max )
            : min_( min ), max_( max ){}

    
    void Write( nMessage& m, REAL value ) const
    {
        clamp( value, min_, max_ );
        unsigned short compressed = static_cast< unsigned short > ( maxShort_ * ( value - min_ )/( max_ - min_ ) );
        m.Write( compressed );
    }

    REAL Read( nMessage& m ) const
    {
        unsigned short compressed;
        m.Read( compressed );

        return  min_ + compressed * ( max_ - min_ )/maxShort_;
    }
private:
    REAL min_, max_;  

    static const unsigned short maxShort_;

    gFloatCompressor();
};

const unsigned short gFloatCompressor::maxShort_ = 0xFFFF;

static gFloatCompressor compressZeroOne( 0, 1 );


void gDestination::WriteCreate(nMessage &m, unsigned short cycle_id ){
    m << position;
    m << direction;
    m << distance;

    unsigned short flags = 0;
    if ( braking )
        flags |= 0x01;
    if ( chatting )
        flags |= 0x02;
    m << flags;

    
    messageID = m.MessageID();

    m.Write( cycle_id );
    m << gameTime;
    m.Write( turns );
}

gDestination *gDestination::RightBefore(gDestination *list, REAL dist){
    if (!list || list->distance > dist)
        return NULL;

    gDestination *ret=list;
    while (ret && ret->next && ret->next->distance < dist)
        ret=ret->next;

    return ret;
}

gDestination *gDestination::RightAfter(gDestination *list, REAL dist){
    if (!list)
        return NULL;

    gDestination *ret=list;
    while (ret && ret->distance < dist)
        ret=ret->next;

    return ret;
}


void gDestination::InsertIntoList(gDestination **l){
    if (!l)
        return;

    RemoveFromList();

    
    while (l && *l && CompareWith( **l ) > 0 )
        l = &((*l)->next);

    list=l;

    tASSERT(l);

    next=*l;
    *l=this;
}



void gDestination::RemoveFromList(){
    

    
    

    if(list)
        *list = next;
    if(next)
        next->list = list;

    next = 0;
    list = 0;
}











REAL gDestination::GetGameTime( void ) const
{
    return this->gameTime;
}












gDestination const & gDestination::GetGameTime( REAL & gameTime ) const
{
    gameTime = this->gameTime;
    return *this;
}












gDestination & gDestination::SetGameTime( REAL gameTime )
{
    this->gameTime = gameTime;
    return *this;
}



static void new_destination_handler(nMessage &m)
{
    
    unsigned short cycle_id;
    gDestination *dest=new gDestination(m, cycle_id );

    
    nNetObject *o=nNetObject::ObjectDangerous(cycle_id);
    if (o && &o->CreatorDescriptor() == &cycle_init){
        if ((sn_GetNetState() == nSERVER) && (m.SenderID() != o->Owner()))
        {
            Cheater(m.SenderID());
        }
        else
            if(o->Owner() != ::sn_myNetID)
            {
                gCycle* c = dynamic_cast<gCycle *>(o);
                if (c)
                {
                    if ( c->Player() && !dest->Chatting() )
                        c->Player()->Activity();

                    
                    if ( dest->GetGameTime() < -100 )
                        dest->SetGameTime( se_GameTime()+c->Lag()*3 );

                    c->AddDestination(dest);
                    dest = 0;
                }
            }
    }

    
    
    delete dest;
}

static nDescriptor destination_descriptor(321,&new_destination_handler,"destinaton");

static void BroadCastNewDestination(gCycleMovement *c, gDestination *dest){
    nMessage *m=new nMessage(destination_descriptor);
    dest->WriteCreate(*m, c->ID() );
    m->BroadCast();
}

bool gCycle::IsMe( eGameObject const * other ) const
{
    return other == this || other == extrapolator_;
}

#ifdef DELAYEDTURN_DEBUG
double sg_turnReceivedTime = 0;
#endif

void gCycle::OnNotifyNewDestination( gDestination* dest )
{
#ifdef DELAYEDTURN_DEBUG
    sg_turnReceivedTime = tSysTimeFloat();
#endif

#ifdef GNUPLOT_DEBUG
    if ( sg_gnuplotDebug && Player() )
    {
        std::ofstream f( Player()->GetUserName() + "_sync", std::ios::app );
        f << dest->position.x << " " << dest->position.y << "\n";
    }
#endif

    gCycleMovement::OnNotifyNewDestination( dest );

    
    if (sn_GetNetState()==nCLIENT && ::sn_myNetID == Owner())
        BroadCastNewDestination(this,dest);

    if ( extrapolator_ )
    {
        
        extrapolator_->NotifyNewDestination( dest );
    }

    
    

    
    if( sn_GetNetState() == nSERVER )
    {
        
        REAL simTime=se_GameTime() - Lag();

        REAL lag = simTime - dest->gameTime;  
        REAL lagOffset = simTime - lastTime;  
        if ( sn_GetNetState() == nSERVER )
        {
            eLag::Report( Owner(), lag );
            if ( currentWall && currentWall->Wall() && rubberSpeedFactor >= 1-EPS )
            {
                lag -= lagOffset; 

                
                if ( lag < 0 )
                    return;

                
                REAL minDist   = currentWall->Wall()->Pos(0);
                REAL maxGoBack = ( distance - minDist ) * .8;

                
                REAL stepBack = distance - dest->distance;

                if ( rubberSpeedFactor < 1-EPS )
                {
                    
                    maxGoBack = stepBack;
                }

                
                if ( stepBack > maxGoBack )
                {
                    stepBack = maxGoBack;
                    lag = rubberSpeedFactor*stepBack/verletSpeed_;
                }

                
                
                lag = eLag::TakeCredit( Owner(), lag + lagOffset ) - lagOffset;

                
                
                
                static nVersionFeature noConfusionFromMoveBack( 16 );
                if( !noConfusionFromMoveBack.Supported( Owner() ) && 
                    lastTime - lag < lastSyncOwnerGameTime_ )
                {
                    lag = lastTime - lastSyncOwnerGameTime_;
                }

                
                if ( lag < 0 )
                    return;

                
                if ( rubberSpeedFactor >= 1-EPS )
                {
                    
                    TimestepCore( lastTime - lag );
                }
                else if ( 0 )
                {
                    

                    
                    REAL step = lag * verletSpeed_;
                    lastTime -= lag;

                    
                    REAL rubberGranted, rubberEffectiveness;
                    sg_RubberValues( player, verletSpeed_, rubberGranted, rubberEffectiveness );
                    if ( rubberEffectiveness > 0 )
                        rubber -= step/rubberEffectiveness;

                    if ( rubber < 0 )
                        rubber = 0;

                    
                    step *= rubberSpeedFactor;
                    distance -= step;
                    pos = pos - dirDrive * step;

                    
                    verletSpeed_ -= acceleration * lag;
                }

                
                if ( distance < minDist )
                {
                    
                    TimestepCore( lastTime + ( minDist - distance )/verletSpeed_ );
                }
            }
        }
    }
}














void gCycle::OnDropTempWall( gPlayerWall * wall, eCoord const & position, eCoord const & dir )
{
    tASSERT( wall );

    unsigned short idrec = ID();
    tRecorderSync< unsigned short >::Archive( "_ON_DROP_WALL", 8, idrec );

    
    bool wallRight = ( currentWall && ( wall->NetWall() == currentWall || wall->NetWall() == lastWall ) );

    
    if ( wallRight && currentWall->Edge()->Vec().NormSquared() < verletSpeed_ * verletSpeed_ * sg_minDropInterval * sg_minDropInterval )
        wallRight = false;

    tRecorderSync< bool >::Archive( "_ON_DROP_WALL_RIGHT", 8, wallRight );

    
    
    if ( wallRight )
    {
        
        REAL alpha = currentWall->Edge()->Edge(0)->Ratio( position );
        tRecorderSync< REAL >::Archive( "_ON_DROP_WALL_ALPHA", 8, alpha );
        if ( alpha > -.5 )
        {
            unsigned short idrec = ID();
            tRecorderSync< unsigned short >::Archive( "_ON_DROP_WALL_DROP", 8, idrec );

            
            dropWallRequested_ = true;

            
            
            lastDirDrive = -dir;
        }
    }
}










void 	gCycle::SetWallsStayUpDelay	( REAL delay )
{
    c_pwsud->Set( delay );
}


static REAL sg_cycleRubberWallShrink = 0;
static nSettingItemWatched<REAL>
sg_cycleRubberWallShrinkConf("CYCLE_RUBBER_WALL_SHRINK",
                             sg_cycleRubberWallShrink,
                             nConfItemVersionWatcher::Group_Bumpy,
                             12);


static REAL sg_cycleDistWallShrink = 0;
static nSettingItemWatched<REAL>
sg_cycleDistWallShrinkConf("CYCLE_DIST_WALL_SHRINK",
                           sg_cycleDistWallShrink,
                           nConfItemVersionWatcher::Group_Bumpy,
                           12);

static REAL sg_cycleDistWallShrinkOffset = 0;
static nSettingItemWatched<REAL>
sg_cycleDistWallShrinkOffsetConf("CYCLE_DIST_WALL_SHRINK_OFFSET",
                                 sg_cycleDistWallShrinkOffset,
                                 nConfItemVersionWatcher::Group_Bumpy,
                                 12);


static REAL sg_CycleWallLengthFromDist( REAL distance )
{
    REAL len = gCycle::WallsLength();
    if ( len <= 0 )
    {
        return len;
    }

    
    REAL d = sg_cycleDistWallShrinkOffset - distance;
    if ( d > 0 )
        len -= sg_cycleDistWallShrink * d;

    return len;
}


REAL gCycle::ThisWallsLength() const
{
    
    REAL len = sg_CycleWallLengthFromDist( distance );
    if ( len <= 0 )
    {
        return len;
    }

    
    return len - GetRubber() * sg_cycleRubberWallShrink;
}



REAL gCycle::WallEndSpeed() const
{
    REAL rubberMax, rubberEffectiveness;
    sg_RubberValues( player, Speed(), rubberMax, rubberEffectiveness );

    
    REAL speed = rubberSpeedFactor * Speed();

    
    REAL d = sg_cycleDistWallShrinkOffset - distance;
    if ( d > 0 )
        speed *= ( 1 - sg_cycleDistWallShrink );

    
    if ( rubberEffectiveness > 0 )
        speed += Speed() * ( 1 - rubberSpeedFactor ) * sg_cycleRubberWallShrink / rubberEffectiveness;

    return speed;
}


REAL gCycle::MaxWallsLength() const
{
    REAL len = sg_CycleWallLengthFromDist( distance );

    
    
    if ( sg_cycleDistWallShrink > 1 )
    {
        len = wallsLength;
    }

    
    if ( sg_cycleRubberWallShrink >= 0 || sg_rubberCycle < 0 )
        return len;
    else
        return len - sg_cycleRubberWallShrink * sg_rubberCycle;
}


void 	gCycle::SetWallsLength			( REAL length)
{
    c_pwl->Set( length );
}


void 	gCycle::SetExplosionRadius		( REAL radius)
{
    c_per->Set( radius );
}




void gCycleExtrapolator::CopyFrom( const gCycleMovement& other )
{
    
    gCycleMovement::CopyFrom( other );

    
    parent_ = &other;

    
    trueDistance_ = GetDistance();
}


void gCycleExtrapolator::CopyFrom( const SyncData& sync, const gCycle& other )
{
    
    gCycleMovement::CopyFrom( sync, other );

    
    MoveSafely( other.lastGoodPosition_, sync.time, sync.time );
    
    MoveSafely( sync.pos, sync.time, sync.time );

#ifdef DEBUG_X
    if ( face1 != face2 || face1 != currentFace )
    {
        currentFace = face1;
        MoveSafely( sync.lastTurn *.01 + sync.pos * .99, sync.time, sync.time );
        MoveSafely( sync.pos, sync.time, sync.time );
    }
#endif

    
    parent_ = &other;

    
    lastTurnPos_ = sync.lastTurn;

    
    trueDistance_ = GetDistance();

    
    
    
    
    if( eLag::Feature().Supported(0) )
    {
        gDestination *dest = GetCurrentDestination();
        if ( dest )
        {
            REAL distToDest = eCoord::F( dest->position - pos, dirDrive );
            if( distToDest < 0 )
            {
                
                
                pos = dest->position;
                lastTime = dest->gameTime;
                distance = dest->distance;
                verletSpeed_ = dest->speed;
            }
        }
    }
}

gCycleExtrapolator::gCycleExtrapolator(eGrid *grid, const eCoord &pos,const eCoord &dir,ePlayerNetID *p,bool autodelete)
        :gCycleMovement( grid, pos, dir, p, autodelete )
        ,trueDistance_( 0 )
        ,parent_( 0 )
{
    
    eFace * currentFaceBack = currentFace;
    RemoveFromList();
    currentFace = currentFaceBack;
    if ( !currentFace )
        currentFace = grid->FindSurroundingFace( pos, currentFace );
}


gCycleExtrapolator::~gCycleExtrapolator()
{
}



bool gCycleExtrapolator::EdgeIsDangerous(const eWall *ww, REAL time, REAL alpha ) const
{
    const gPlayerWall *w = dynamic_cast<const gPlayerWall*>(ww);
    if (w)
    {
        gNetPlayerWall* nw = w->NetWall();

        
        REAL builtTime = w->Time(alpha);

        
        if ( builtTime > time )
            return false;

        
        if ( nw && nw->Preliminary() && w->Cycle() == parent_ && fabs( dirDrive * w->Vec() ) < EPS )
            return false;

        
        gCycle *otherPlayer=w->Cycle();
        if (otherPlayer==parent_ &&
                ( time < builtTime + 2 * GetTurnDelay() || GetDistance() < w->Pos( alpha ) + .01 * sg_delayCycle*Speed()/SpeedMultiplier()  )
           )
            return false;
    }

    
    return bool(parent_) && parent_->EdgeIsDangerous( ww, time, alpha ) && gCycleMovement::EdgeIsDangerous( ww, time, alpha );
}

void gCycleExtrapolator::PassEdge(const eWall *ww,REAL time,REAL a,int){
    {
        if (!EdgeIsDangerous(ww,time,a) || !Alive() )
        {
            return;
        }
        else
        {
            eCoord collPos = ww->Point( a );
            throw gCycleDeath( collPos );
        }
    }
}

bool gCycleExtrapolator::TimestepCore(REAL currentTime, bool calculateAcceleration)
{
    
    gDestination destDefault( *parent_ ), *dest=GetCurrentDestination();
    if ( !dest )
    {
        dest = &destDefault;
    }

    
    
    
    tASSERT(std::isfinite(distance));

    
    bool ret = false;
    try{
        ret = gCycleMovement::TimestepCore( currentTime, calculateAcceleration );
    }
    catch( gCycleDeath & )
    {
        return false;
    }

    
    
    trueDistance_ = distance;

    return ret;
}



nDescriptor &gCycleExtrapolator::CreatorDescriptor() const{
    
    tASSERT( 0 );
    return cycle_init;
}



const eTempEdge* gCycle::Edge(){
    if (currentWall)
        return currentWall->Edge();
    else
        return NULL;
}

const gPlayerWall* gCycle::CurrentWall(){
    if (currentWall)
        return currentWall->Wall();
    else
        return NULL;
}






#ifndef DEDICATED
struct gCycleVisuals
{
    rModel *customModel, *bodyModel, *frontModel, *rearModel; 
    gTextureCycle *customTexture, *bodyTexture, *wheelTexture; 
    gRealColor color; 
    bool mpType;      
    int  mpPreference; 

    gCycleVisuals( gRealColor const & a_color )
    {
        customModel = bodyModel = frontModel = rearModel = 0;
        customTexture = bodyTexture = wheelTexture = 0;

        color = a_color;

        mpType = false;
        mpPreference = 0;
    }

    ~gCycleVisuals()
    {
        delete customTexture;
        delete bodyTexture;
        delete wheelTexture;
    }

    enum Slot{ SLOT_CUSTOM=0, SLOT_BODY=1, SLOT_WHEEL=2, SLOT_MAX=3 };

    
    static rSurface * LoadTextureSafe2( Slot slot, int mp )
    {
        static std::unique_ptr<rSurface> cache[SLOT_MAX][2];
        std::unique_ptr<rSurface> & surface = cache[slot][mp];
        if ( surface.get() == NULL )
        {
            static char const * names[SLOT_MAX]={"bike.png","cycle_body.png", "cycle_wheel.png"};
            char const * name = names[slot];

            char const * folder = mp ? "moviepack" : "textures";
            tString file = tString(folder) + "/" + name;

            surface.reset( tNEW( rSurface( file ) ) );
        }

        if ( surface->GetSurface() )
            return surface.get();
        else
            return NULL;
    }

    
    gTextureCycle * LoadTextureSafe( Slot slot, bool wheel )
    {
        rSurface * surface = LoadTextureSafe2( slot, mpPreference );
        if ( !surface )
            surface = LoadTextureSafe2( slot, 1-mpPreference );

        if ( surface )
            return tNEW( gTextureCycle )( *surface, color, 0, 0, wheel );

        return NULL;
    }

    
    bool LoadTextures()
    {
        if ( mpType )
        {
            if ( !customTexture )
                customTexture = LoadTextureSafe( SLOT_CUSTOM, false );

            return customTexture;
        }
        else
        {
            if ( !bodyTexture )
                bodyTexture = LoadTextureSafe( SLOT_BODY, false );
            if ( !wheelTexture )
                wheelTexture = LoadTextureSafe( SLOT_WHEEL, true );

            return bodyTexture && wheelTexture;
        }
    }

    
    static rModel * LoadModelSafe( char const * filename )
    {
        return rModel::GetModel(filename);
    }

    
    bool LoadModel( bool a_mpType, bool mpFolder )
    {
        mpType = a_mpType;
        char const * folder = mpFolder ? "moviepack" : "models";

        if ( mpType )
        {
            tString base = tString(folder);
            base << "/cycle";
            if (!customModel) customModel = LoadModelSafe( base + ".ASE" );
            if (!customModel) customModel = LoadModelSafe( base + ".ase" );

            return customModel && LoadTextures();
        }
        else
        {
            tString base = tString(folder) + "/cycle_";

            if (!bodyModel) bodyModel = LoadModelSafe( base + "body.mod" );
            if (!frontModel) frontModel = LoadModelSafe( base + "front.mod" );
            if (!rearModel) rearModel = LoadModelSafe( base + "rear.mod" );

            return bodyModel && frontModel && rearModel && LoadTextures();
        }
    }

    
    bool LoadModel2( bool mp )
    {
        
        return LoadModel( mp, mp ) || LoadModel( !mp, mp );
    }

    
    bool LoadModel( bool mp )
    {
        mpPreference = mp ? 1 : 0;

        
        return LoadModel2( mp ) || LoadModel2( !mp );
    }
};
#endif

#ifndef DEDICATED

class gCycleWallRenderer: public eReferencableGameObject
{
public:
    gCycleWallRenderer( gCycle * cycle )
    : eReferencableGameObject( cycle->Grid(), cycle->Position(), cycle->Direction(), cycle->CurrentFace(), true )
    , cycle_( cycle )
    {
        AddToList();
    }

#if 0 
    virtual ~gCycleWallRenderer()
    {
    }

    virtual void OnRemoveFromGame()
    {
        eReferencableGameObject::OnRemoveFromGame();
    }
#endif
private:
    virtual void Render( eCamera const * camera )
    {
        cycle_->displayList_.RenderAll( camera, cycle_ );
    }

    virtual bool Timestep( REAL currentTime )
    {
        if ( !cycle_ )
        {
            return true;
        }

        Move( cycle_->Position(), lastTime, currentTime );

        return !cycle_->Alive() && !cycle_->displayList_.Walls();
    }

    tJUST_CONTROLLED_PTR< gCycle > cycle_;
};
#endif

void gCycle::MyInitAfterCreation(){

#ifndef DEDICATED
    new gCycleWallRenderer( this );
#endif

    dropWallRequested_ = false;
    lastGoodPosition_ = pos;
    lastTexR_ = -1.f;
    lastTexG_ = -1.f;
    lastTexB_ = -1.f;
    lastTexUpdateTime_ = 0.0;

    
    modPeakRubberInSpot = 0.0f;
    modWasUsingRubber = false;
    modLastRubberAmount = 0.0f;
    modLastWaypointTime = 0.0f;
    modRubberAtEventStart = 0.0f;
    modSpotPosition = pos;

    
    intentTurnLeft = false;
    intentTurnRight = false;
    intentAutoEscapeDir = 0;
    previousFrameRubber = 0.0f;

    
    rightTurnCount = 0;
    leftTurnCount = 0;
    for (int i = 0; i < 3; i++) {
        rightTurnTimes[i] = 0.0f;
        leftTurnTimes[i] = 0.0f;
    }

#ifdef DEBUG
    
#endif
    engine  = tNEW(eSoundPlayer)(cycle_run,true);
    turning = tNEW(eSoundPlayer)(turn_wav);
    spark   = tNEW(eSoundPlayer)(scrap);

    
    correctDistanceSmooth = 0;

    resimulate_ = false;

    mp=sg_MoviePack();

    lastTimeAnim = 0;
    timeCameIntoView = 0;

    customModel = NULL;
    body = NULL;
    front = NULL;
    rear = NULL;
    wheelTex = NULL;
    bodyTex = NULL;
    customTexture = NULL;

    correctPosSmooth=eCoord(0,0);

    rotationFrontWheel=rotationRearWheel=eCoord(1,0);

    skew=skewDot=0;

    {
        dir=dirDrive;
        REAL dirLen=dir.NormSquared();
        if ( dirLen > 0 )
        {
            dir = dir * (1/sqrt( dirLen ));
        }
    }

    if (sn_GetNetState()!=nCLIENT){
        if(!player)
        { 
            
            color_.r=1;
            color_.g=1;
            color_.b=1;

            trailColor_.r=1;
            trailColor_.g=1;
            trailColor_.b=1;
        }
        else
        {
            player->Color(color_.r,color_.g,color_.b);
            player->TrailColor(trailColor_.r,trailColor_.g,trailColor_.b);
        }

        se_MakeColorValid( color_.r, color_.g, color_.b, 1.0f );
        se_MakeColorValid( trailColor_.r, trailColor_.g, trailColor_.b, .5f );
    }

    
#ifndef DEDICATED
    gCycleVisuals visuals( color_ );
    if ( !visuals.LoadModel( mp ) )
    {
        tERR_ERROR( "Neither classic style nor moviepack style model and textures found. "
                    "The folders \"textures\" and \"moviepack\" need to contain either "
                    "cycle.ase and bike.png or body.mod, front.mod, rear.mod, cycle_body.png and cycle_wheel.png." );
    }

    mp = visuals.mpType;

    
    if ( mp )
    {
        
        customModel = visuals.customModel;
        visuals.customModel = 0;
        customTexture = visuals.customTexture;
        visuals.customTexture = 0;
    }
    else
    {
        
        body = visuals.bodyModel;
        visuals.bodyModel = 0;
        front = visuals.frontModel;
        visuals.frontModel = 0;
        rear = visuals.rearModel;
        visuals.rearModel = 0;
        bodyTex = visuals.bodyTexture;
        visuals.bodyTexture = 0;
        wheelTex = visuals.wheelTexture;
        visuals.wheelTexture = 0;

        tASSERT ( body && front && rear && bodyTex && wheelTex );

        mp = false;
    }
#endif 

    

#ifdef DEBUG
    
#endif
    nextSyncOwner=nextSync=tSysTimeFloat()-1;
    lastSyncOwnerGameTime_ = 0;

    if (sn_GetNetState()!=nCLIENT)
        RequestSync();

    grid->AddGameObjectInteresting(this);

    spawnTime_=lastTimeAnim=lastTime;
    
    if ( !sg_cycleFirstSpawnProtection && spawnTime_ <= 1.0 )
    {
        spawnTime_ = -1E+20;
    }

    if ( engine )
        engine->Reset(10000);

    if ( turning )
        turning->End();

    
    this->AddToList();

    predictPosition_ = pos;

#ifdef GNUPLOT_DEBUG
    if ( sg_gnuplotDebug && Player() )
    {
        std::ofstream f( Player()->GetUserName() + "_step" );
        f << pos.x << " " << pos.y << "\n";
        std::ofstream g( Player()->GetUserName() + "_sync" );
        g << pos.x << " " << pos.y << "\n";
        std::ofstream h( Player()->GetUserName() + "_turn" );
        h << pos.x << " " << pos.y << "\n";
    }
#endif
}

void gCycle::InitAfterCreation(){
#ifdef DEBUG
    if (!std::isfinite(Speed()))
        st_Breakpoint();
#endif
    gCycleMovement::InitAfterCreation();
#ifdef DEBUG
    if (!std::isfinite(Speed()))
        st_Breakpoint();
#endif
    MyInitAfterCreation();
}

gCycle::gCycle(eGrid *grid, const eCoord &pos,const eCoord &d,ePlayerNetID *p)
        :gCycleMovement(grid, pos,d,p,false),
        engine(NULL),
        turning(NULL),
        skew(0),skewDot(0),
        rotationFrontWheel(1,0),rotationRearWheel(1,0),heightFrontWheel(0),heightRearWheel(0),
        currentWall(NULL),
        lastWall(NULL)
{
    windingNumberWrapped_ = windingNumber_ = Grid()->DirectionWinding(dirDrive);
    dirDrive = Grid()->GetDirection(windingNumberWrapped_);
    dir = dirDrive;

    deathTime=0;

    lastNetWall=lastWall=currentWall=NULL;

    MyInitAfterCreation();

    
    intentTurnLeft = false;
    intentTurnRight = false;
    intentAutoEscapeDir = 0;
    debug_GapTimerTicks = 0;
    debug_lastTurnDir = 0;
    previousFrameRubber = 0.0f;

    
    rightTurnCount = 0;
    leftTurnCount = 0;
    for (int i = 0; i < 3; i++) {
        rightTurnTimes[i] = 0.0f;
        leftTurnTimes[i] = 0.0f;
    }

    sg_ArchiveCoord( this->dirDrive, 1 );
    sg_ArchiveCoord( this->dir, 1 );
    sg_ArchiveCoord( this->pos, 1 );
    sg_ArchiveReal( this->verletSpeed_, 1 );
}

gCycle::~gCycle(){
#ifdef DEBUG
    
#endif
    

    tDESTROY(engine);
    tDESTROY(turning);
    tDESTROY(spark);

    this->RemoveFromGame();

    if (mp){
        delete customTexture;
    }
    else{
        delete wheelTex;
        delete bodyTex;
    }
#ifdef DEBUG
    
#endif
    
}

void gCycle::OnRemoveFromGame()
{
    
    tJUST_CONTROLLED_PTR< gCycle > keep;

    if ( this->GetRefcount() > 0 )
    {
        keep = this;

        this->Turn(0);

        if ( sn_GetNetState() == nSERVER )
            RequestSync();
    }

    
    if ( Alive() )
    {
        Die( lastTime );
    }

    if (currentWall)
        currentWall->CopyIntoGrid(0);
    currentWall=NULL;
    lastWall=NULL;

    gCycleMovement::OnRemoveFromGame();
}


void gCycle::OnRoundEnd()
{
    
    if ( Alive() && player )
    {
        Player()->AddScore( score_survive, tOutput("$player_win_survive"), tOutput() );
    }
}


static inline void rotate(eCoord &r,REAL angle){
    REAL x=r.x;
    r.x+=r.y*angle;
    r.y-=x*angle;
    r=r*(1/sqrt(r.NormSquared()));
}

#ifdef MACOSX

bool crash_sparks=false;
#else
bool crash_sparks=true;
#endif




extern REAL planned_rate_control[MAXCLIENTS+2];


struct gPredictPositionData
{
    REAL maxSpaceReport; 
#ifdef DEDICATED
    REAL rubberStart;    
#endif
};










void gCycle::PreparePredictPosition( gPredictPositionData & data )
{
    
    data.maxSpaceReport = 0;
#ifdef DEDICATED
    if ( sg_predictWalls )
    {
        
        REAL maxTime = lastTime + MaxSimulateAhead() + GetMaxLazyLag();
        REAL predictDT = maxTime - lastTime;
        REAL lookAhead = predictDT * verletSpeed_;

        
        data.rubberStart = verletSpeed_ / RubberSpeed();

        
        data.maxSpaceReport = lookAhead + data.rubberStart;
    }
#else
    data.maxSpaceReport = verletSpeed_ * se_PredictTime() * rubberSpeedFactor;
#endif

    
    maxSpaceMaxCast_ = data.maxSpaceReport;
}












REAL gCycle::CalculatePredictPosition( gPredictPositionData & data )
{
    
    REAL predictTime = lastTime;
    {
#ifdef DEDICATED
        predictPosition_ = pos;

        if ( sg_predictWalls )
        {
            REAL spaceAhead = GetMaxSpaceAhead( data.maxSpaceReport );
            spaceAhead -= data.rubberStart;

            if ( spaceAhead > 0 )
            {
                
                predictPosition_ = pos + dirDrive * spaceAhead;
                predictTime = lastTime + spaceAhead/verletSpeed_;
            }
        }
#else
        
        predictTime += se_PredictTime();
        predictPosition_ = pos+correctPosSmooth + dirDrive * GetMaxSpaceAhead( data.maxSpaceReport );
#endif
    }

    return predictTime;
}

bool gCycle::Timestep(REAL currentTime){
    if (Player()) {
        Player()->Color(color_.r, color_.g, color_.b);
        Player()->TrailColor(trailColor_.r, trailColor_.g, trailColor_.b);
    }

    
    gMaxSpaceAheadHitInfoClearer hitInfoClearer( maxSpaceHit_ );

    
    REAL rubberSpeedFactorBack = rubberSpeedFactor;

    
    

    
    gPredictPositionData predictPositionData;
    PreparePredictPosition( predictPositionData );

    
    if ( dropWallRequested_ )
    {
        
        static double nextDrop = 0;
        double time = tSysTimeFloat();
        if ( time >= nextDrop )
        {
            unsigned short idrec = ID();
            tRecorderSync< unsigned short >::Archive( "_PARTIAL_COPY_GRID", 8, idrec );

            nextDrop = time + sg_minDropInterval;
            if ( currentWall )
            {
                currentWall->Update(lastTime,pos);
                currentWall->PartialCopyIntoGrid( grid );
            }
            dropWallRequested_ = false;
        }
    }

#ifdef GNUPLOT_DEBUG
    if ( sg_gnuplotDebug && Player() )
    {
        std::ofstream f( Player()->GetUserName() + "_step", std::ios::app );
        f << pos.x << " " << pos.y << "\n";
    }
#endif
    
    
    

    
    if ( !Alive() )
    {
        
        Die( lastTime );

        
        
        return true;
    }

    
    REAL autoEscapeDt = currentTime - lastTime;

    
    sg_ArchiveCoord( pos, 7 );
    sg_ArchiveReal( verletSpeed_, 7 );

    if ( !destinationList && sn_GetNetState() == nCLIENT )
    {
        
        gDestination* dest = tNEW(gDestination)(*this);
        dest->messageID = 0;
        dest->distance = -.001;
        dest->InsertIntoList(&destinationList);
    }

    
    if ( !extrapolator_ && resimulate_ )
        ResetExtrapolator();

    
    if ( extrapolator_ )
    {
        REAL dt = ( currentTime - lastTime ) * sg_syncFF / sg_syncFFSteps;
#ifdef DEBUG
        
        
        
#endif
        for ( int i = sg_syncFFSteps - 1; i>= 0; --i )
        {
            if ( Extrapolate( dt ) )
            {
                SyncFromExtrapolator();
                break;
            }
        }
    }

    bool ret = false;

    
    if (currentTime < lastTime)
    {
        ret = gCycleMovement::Timestep(currentTime);
    }
    
    else if ( !currentDestination && pendingTurns.empty() )
    {
        
        
        
        if (false && (intentTurnLeft || intentTurnRight) && Alive()) {
            int tDir = intentTurnRight ? 1 : -1;

            gSensor fwdSensor(this, pos, dirDrive);
            fwdSensor.detect(200.0f);
            REAL distToWallFront = fwdSensor.hit;

            
            eCoord wallPoint = pos + dirDrive * distToWallFront;
            eDebugLine::SetTimeout(0.15f);
            eDebugLine::SetColor(0.0f, 1.0f, 1.0f);
            eDebugLine::Draw(wallPoint, 0.5f, wallPoint, 8.0f);

            int crossWN = windingNumberWrapped_;
            Grid()->Turn(crossWN, 1);
            eCoord crossVec = Grid()->GetDirection(crossWN);
            eDebugLine::SetColor(1.0f, 1.0f, 0.0f);
            eDebugLine::Draw(wallPoint + crossVec * 2.0f, 4.0f,
                             wallPoint - crossVec * 2.0f, 4.0f);

            eDebugLine::SetColor(0.2f, 0.3f, 0.8f);
            eDebugLine::Draw(pos, 2.0f, wallPoint, 2.0f);

            if (distToWallFront < 5.0f) {
                REAL urgency = 1.0f - (distToWallFront / 5.0f);
                eDebugLine::SetColor(urgency, 1.0f - urgency, 0.0f);
                eDebugLine::Draw(pos, 3.0f, wallPoint, 3.0f);
            }

            REAL rubberNow = GetRubber();
            REAL rubberDelta = rubberNow - previousFrameRubber;

            bool flush = (distToWallFront < 0.05f);
            bool rubberClose = (rubberDelta > 0.001f && distToWallFront < 0.3f);
            bool calibrationTrigger = (sg_perfectTurnCalibration > 0.01f &&
                                       distToWallFront <= sg_perfectTurnCalibration);

            if (flush || rubberClose || calibrationTrigger) {
                Turn(tDir);
                debug_lastTurnDir = tDir;
                debug_GapTimerTicks = 5;
                intentTurnLeft = false;
                intentTurnRight = false;

                char buf[160];
                snprintf(buf, sizeof(buf),
                    "0xffff44[ SEAL ] dist=%.6f rub=%.2f trigger=%s\n",
                    distToWallFront, rubberNow,
                    flush ? "FLUSH" : (rubberClose ? "RUBBER-CLOSE" : "CALIBRATION"));
                con << buf;
            }
        }

        previousFrameRubber = GetRubber();

        
        if (debug_GapTimerTicks > 0) {
            debug_GapTimerTicks--;
            if (debug_GapTimerTicks == 0 && Alive()) {
                int measureDir = -debug_lastTurnDir;
                int measureWN = windingNumberWrapped_;
                Grid()->Turn(measureWN, measureDir);
                eCoord measureVec = Grid()->GetDirection(measureWN);

                gSensor sideSensor(this, pos, measureVec);
                sideSensor.detect(50.0f);
                REAL gapToWallSide = sideSensor.hit;

                char buf[128];
                snprintf(buf, sizeof(buf),
                    "0x44ffff[ SEAL RESULT ] Gap: %.6f | Rubber used: %.2f\n",
                    gapToWallSide, GetRubber());
                con << buf;

                eDebugLine::SetTimeout(2.0f);
                if (gapToWallSide < 0.01f)
                    eDebugLine::SetColor(0.0f, 1.0f, 0.0f);
                else if (gapToWallSide < 0.5f)
                    eDebugLine::SetColor(1.0f, 1.0f, 0.0f);
                else
                    eDebugLine::SetColor(1.0f, 0.0f, 0.0f);
                eDebugLine::Draw(pos, 1.0f, pos + measureVec * gapToWallSide, 4.0f);
            }
        }

        
        if ( bool(player) &&
                player->IsHuman() &&
                ( sg_chatBotAlwaysActive || player->IsChatting() ) &&
                player->Owner() == sn_myNetID )
        {
            gCycleChatBot & bot = gCycleChatBot::Get( this );
            bot.Activate( currentTime );
        }
        else if ( chatBot_.get() )
        {
            chatBot_->nextChatAI_ = 0;
        }

        bool simulate=Alive();

        if ( !pendingTurns.empty() || currentDestination )
            simulate=true;

        if (simulate)
        {
            try
            {
                ret = gCycleMovement::Timestep(currentTime);
            }
            catch ( gCycleDeath const & death )
            {
                KillAt( death.pos_ );
                return false;
            }
        }
        else
            ret = !Alive();
    }
    else
    {
        
        try
        {
            gCycleMovement::Timestep(currentTime);
        }
        catch ( gCycleDeath const & death )
        {
            KillAt( death.pos_ );
            return false;
        }
    }

    
    try
    {
        if ( currentTime > lastTime )
            ret = gCycleMovement::Timestep(currentTime);
    }
    catch ( gCycleDeath const & death )
    {
        KillAt( death.pos_ );
        return false;
    }

    REAL predictTime = CalculatePredictPosition( predictPositionData );

    if ( Alive() && currentWall )
    {
        
        
        

        
        currentWall->Update(predictTime, PredictPosition() );
    }

    
    if ( currentWall )
    {
        if ( rubberSpeedFactor >= .99 && rubberSpeedFactorBack < .99 )
        {
            currentWall->Checkpoint();
        }
        else if ( rubberSpeedFactor < .99 && rubberSpeedFactorBack >= .99 )
        {
            currentWall->Checkpoint();
        }
        else if ( rubberSpeedFactor < .1 && rubberSpeedFactorBack >= .1 )
        {
            currentWall->Checkpoint();
        }
        else if ( rubberSpeedFactor < .01 && rubberSpeedFactorBack >= .01 )
        {
            currentWall->Checkpoint();
        }
    }

    if ( sn_GetNetState()==nSERVER )
    {
        
        if ( rubberSpeedFactor < .99 && rubberSpeedFactorBack >= .99 )
        {
            RequestSyncOwner();
        }
    }

    
#ifdef DEDICATED
    if ( Alive() && currentTime > lastTime + Lag() + 1 )
    {
        sn_ConsoleOut( "0xff7777Admin : 0xffff77BUG had to kill a cycle because it lagged behind in the simulation. Probably the invulnerability bug. Investigate!\n" );
        st_Breakpoint();
        if( Vulnerable() )
            KillAt( pos );
        else
            Kill();
        ret = false;
    }
#endif

    
    extern REAL sg_rubberCycle;
    if (false && intentAutoEscapeDir != 0 && Alive()) {
        REAL currentRubber = GetRubber();
        
        
        if (autoEscapeDt > 0.0001f) {
            REAL rubberDelta = currentRubber - previousFrameRubber;
            
            
            if (rubberDelta > 0.001f) {
                REAL rubberBurnRate = rubberDelta / autoEscapeDt;
                REAL rubberLeft = sg_rubberCycle - currentRubber - sg_autoEscapeRubberMargin; 
                if (rubberLeft < 0.0f) rubberLeft = 0.0f;
                
                REAL timeToDeath = rubberLeft / rubberBurnRate;
                
                
                REAL pingSeconds = 0.0f;
                if (sg_autoEscapePingComp && sn_GetNetState() == nCLIENT) {
                    pingSeconds = sn_Connections[0].ping.GetPing(); 
                }
                
                
                REAL requiredReactionTime = pingSeconds * 0.5f;
                
                if (timeToDeath <= requiredReactionTime) {
                    Turn(intentAutoEscapeDir);
                    intentAutoEscapeDir = 0; 
                    con << "0xffff44[ AUTO-ESCAPE: PERFECT JUMP ]\n";
                }
            }
        }
    }

    
    previousFrameRubber = GetRubber();
    return ret;
}


static void DecaySmooth( REAL& smooth, REAL relSpeed, REAL minSpeed, REAL clamp )
{
#ifdef DEBUG
    if ( fabs(smooth) > .01 )
    {
        int x;
        x = 1;
    }
#endif

    
    if ( clamp > 0 )
        relSpeed *= ( 1 + smooth * smooth / ( clamp * clamp ) );

    
    if ( smooth == 0 )
    {
        return;
    }

    
    REAL speed = smooth * relSpeed;

    
    if ( fabs( speed ) < minSpeed )
        speed = std::copysign ( minSpeed , smooth );

    
    if ( fabs( speed ) > fabs( smooth ) )
        smooth = 0;
    else
        smooth -= speed;
}


static REAL ClampDisplacement( gCycle* cycle, eCoord& displacement, const eCoord& lookout, const eCoord& pos )
{
    gSensor sensor( cycle, pos, lookout );
    sensor.detect(1);
    if ( sensor.ehit && sensor.hit >= 0 && sensor.hit < 1 )
    {
#ifdef DEBUG_X
        
        gSensor sensor( cycle, pos, lookout );
        sensor.detect(1);
#endif
        displacement = displacement * sensor.hit;
    }
    return sensor.hit;
}


REAL sg_GetSparksDistance();


bool gCycle::TimestepCore(REAL currentTime, bool calculateAcceleration ){
    if (!std::isfinite(skew))
        skew=0;
    if (!std::isfinite(skewDot))
        skewDot=0;

    

    REAL ts=(currentTime-lastTime);

    clamp(ts, -10, 10);
    
    clamp(correctDistanceSmooth, -100, 100);
    

    
    
    

    REAL smooth = 0;

    if ( ts > 0 )
    {
        
        smooth = 1 - 1/( 1 + ts / sg_cycleSyncSmoothTime );
    }

    
    
    
    
    
    

    
    TransferPositionCorrectionToDistanceCorrection();

    
    
    

    

    

    REAL animts=currentTime-lastTimeAnim;
    if (animts<0 || !std::isfinite(animts))
        animts=0;
    else
        lastTimeAnim=currentTime;

    
    {
        
        REAL minSpeed = sg_cycleSyncSmoothMinSpeed * Speed() * animts;
        REAL clamp    = sg_cycleSyncSmoothThreshold * Speed();

        DecaySmooth( correctPosSmooth.x, smooth, minSpeed, clamp );
        DecaySmooth( correctPosSmooth.y, smooth, minSpeed, clamp );

        
        if ( correctPosSmooth.NormSquared() > EPS )
        {
            
            ClampDisplacement( this, correctPosSmooth, correctPosSmooth * 2, pos );

            
            ClampDisplacement( this, correctPosSmooth, -correctPosSmooth, pos );

            
            eCoord lookahead = dirDrive * ( 2 * correctPosSmooth.Norm() );
            ClampDisplacement( this, correctPosSmooth, lookahead, pos );
        }
    }

    if (animts>.2)
        animts=.2;

    rotate(rotationFrontWheel,2*verletSpeed_*animts/.43);
    rotate(rotationRearWheel,2*verletSpeed_*animts/.73);

    REAL sparksDistance = sg_GetSparksDistance();
    REAL extension = .25;
    if ( extension < sparksDistance )
        extension = sparksDistance;

    

    
    {
        
        dir=dir+dirDrive*animts*verletSpeed_*3;

        
        eCoord dirDistance = dir - dirDrive;
        REAL dist = dirDistance.NormSquared();
        const REAL maxDist = .8;
        if (dirDistance.NormSquared() > maxDist)
            dir = dirDrive + dirDistance* (1/sqrt(dist/maxDist));

        
        dir=dir*(1/sqrt(dir.NormSquared()));
    }

    {
        eCoord oldPos = pos;

        if ( Alive() ){
            
            try
            {
                
                REAL startBuildWallAt = spawnTime_ + sg_cycleWallTime;
                if ( !currentWall && currentTime > startBuildWallAt  )
                {
                    
                    if ( currentTime < startBuildWallAt )
                        if ( gCycleMovement::TimestepCore( startBuildWallAt, calculateAcceleration ) )
                            return true;
                    calculateAcceleration = true;

                    
                    REAL lastSpawn = spawnTime_;
                    spawnTime_ += -1E+20;
                    DropWall();
                    spawnTime_ = lastSpawn;
                    lastTurnPos_ = pos; 
                }

                
                if ( gCycleMovement::TimestepCore( currentTime, calculateAcceleration ) )
                    return true;
            }
            catch ( gCycleDeath const & death )
            {
                KillAt( death.pos_ );

                
                oldPos = death.pos_;
            }
        }

        
        if ( !Alive() )
        {
            MoveSafely(oldPos,currentTime,currentTime);
            Die( currentTime );
        }
    }

    if (Alive() && sg_modWaypointsEnabled) {
        
        REAL currentRubber = GetRubber();
        if (currentRubber > modLastRubberAmount + 0.001f) {
            
            if (!modWasUsingRubber) {
                modWasUsingRubber = true;
                modPeakRubberInSpot = currentRubber;
                modRubberAtEventStart = currentRubber;
                modSpotPosition = this->pos;
            } else {
                
                if (currentRubber > modPeakRubberInSpot) {
                    modPeakRubberInSpot = currentRubber;
                }
            }
        } else if (modWasUsingRubber && currentRubber < modLastRubberAmount) {
            
            bool shouldSpawn = (modPeakRubberInSpot >= sg_modWaypointsMinRubber);

            
            if (shouldSpawn && (se_GameTime() - modLastWaypointTime < sg_modWaypointsCooldown)) {
                shouldSpawn = false;
            }

            
            if (shouldSpawn && !sg_modWaypointsEnemyWalls) {
                
                gSensor fwd(this, modSpotPosition, dirDrive);
                fwd.detect(1.0f);
                if (fwd.ehit) {
                    gPlayerWall *pw = dynamic_cast<gPlayerWall*>(fwd.ehit->GetWall());
                    if (pw) {
                        
                        
                        shouldSpawn = false;
                    }
                }
            }

            if (shouldSpawn) {
                gRealColor wpColor;
                REAL maxRubber = sg_rubberCycle;
                REAL ratio = (maxRubber > 0.001f) ? (modPeakRubberInSpot / maxRubber) : 0.0f;
                if (ratio > 1.0f) ratio = 1.0f;
                wpColor.r = ratio < 0.5f ? (ratio * 2.0f) : 1.0f;
                wpColor.g = ratio > 0.5f ? ((1.0f - ratio) * 2.0f) : 1.0f;
                wpColor.b = 0.0f;
                
                new gRubberWaypointObject(this->Grid(), modSpotPosition, modPeakRubberInSpot, wpColor);
                modLastWaypointTime = se_GameTime();

                
                REAL rubberEventVal = modPeakRubberInSpot - modRubberAtEventStart;
                if (this->Player()) {
                    tColoredString playerName;
                    playerName << this->Player()->GetUserName();
                    con << "Player " << playerName << ": Rubber event " << rubberEventVal << "\n";
                }
            }
            modWasUsingRubber = false;
            modPeakRubberInSpot = 0.0f;
        }
        modLastRubberAmount = currentRubber;
    }

    
    sg_ArchiveCoord( pos, 7 );
    sg_ArchiveReal( verletSpeed_, 7 );

    if (Alive()){
#ifndef DEDICATED
        
        gSensor fl(this,pos,dirDrive.Turn(1,1));
        gSensor fr(this,pos,dirDrive.Turn(1,-1));

        fl.detect(extension*4);
        fr.detect(extension*4);

        if (fl.hit > extension)
            fl.hit = extension;

        if (fr.hit > extension)
            fr.hit = extension;

        REAL lr=(fl.hit-fr.hit)/extension;

        
        const REAL skewrelax=24;
        skewDot-=128*(skew+lr/2)*animts;
        skewDot/=1+skewrelax*animts;
        if (skew*skewDot>4) skewDot=4/skew;
        skew+=skewDot*animts;

        REAL fac = 0.5f;
        if ( skew > fr.hit * fac )
        {
            skew = fr.hit * fac;
        }
        if ( skew < -fl.hit * fac )
        {
            skew = -fl.hit * fac;
        }

        
        eCoord sparkpos,sparkdir;

        if (fl.ehit && fl.hit<=sparksDistance){
            sparkpos=pos+dirDrive.Turn(1,1)*fl.hit;
            sparkdir=dirDrive.Turn(0,-1);
        }
        if (fr.ehit && fr.hit<=sparksDistance){
            sparkpos=pos+dirDrive.Turn(1,-1)*fr.hit;
            sparkdir=dirDrive.Turn(0,1);
        }

        if (fabs(skew)<fabs(lr*.8) ){
            skewDot-=lr*1000*animts;
            if (crash_sparks && animts>0)
            {
                gPlayerWall *tmpplayerWall=0;

                if(fl.ehit) tmpplayerWall=dynamic_cast<gPlayerWall*>(fl.ehit->GetWall());
                if(fr.ehit) tmpplayerWall=dynamic_cast<gPlayerWall*>(fr.ehit->GetWall());

                if(tmpplayerWall) {
                    gCycle *tmpcycle = tmpplayerWall->Cycle();

                    if( tmpcycle )
                        new gSpark(grid, sparkpos-dirDrive*.1,sparkdir,currentTime,color_.r,color_.g,color_.b,tmpcycle->color_.r,tmpcycle->color_.g,tmpcycle->color_.b);
                }
                else
                    new gSpark(grid, sparkpos-dirDrive*.1,sparkdir,currentTime,color_.r,color_.g,color_.b,1,1,1);

                if ( spark )
                    spark->Reset();
            }
        }

        if (fabs(skew)<fabs(lr*.9) ){
            skewDot-=lr*100*animts;
            if (crash_sparks && animts>0)
            {
                gPlayerWall *tmpplayerWall=0;

                if(fl.ehit) tmpplayerWall=dynamic_cast<gPlayerWall*>(fl.ehit->GetWall());
                if(fr.ehit) tmpplayerWall=dynamic_cast<gPlayerWall*>(fr.ehit->GetWall());

                if(tmpplayerWall) {
                    gCycle *tmpcycle = tmpplayerWall->Cycle();

                    if( tmpcycle )
                        new gSpark(grid, sparkpos-dirDrive*.1,sparkdir,currentTime,color_.r,color_.g,color_.b,tmpcycle->color_.r,tmpcycle->color_.g,tmpcycle->color_.b);
                }
                else
                    new gSpark(grid, sparkpos-dirDrive*.1,sparkdir,currentTime,color_.r,color_.g,color_.b,1,1,1);
            }
        }
#endif
        
    }

    
    if (skew>.5){
        skew=.5;
        skewDot=0;
    }

    if (skew<-.5){
        skew=-.5;
        skewDot=0;
    }

    if ( sn_GetNetState()==nSERVER )
    {
        if (nextSync < tSysTimeFloat() )
        {
            
            REAL lookahead = Speed() * sg_syncIntervalEnemy*.5;
            if ( !sg_avoidBadOldClientSync || sg_NoLocalTunnelOnSync.Supported( Owner() ) || GetMaxSpaceAhead( lookahead ) >= lookahead )
            {
                RequestSync(false);

                
                if ( currentWall )
                    currentWall->Checkpoint();
            }

            nextSync=tSysTimeFloat()+sg_syncIntervalEnemy;
            nextSyncOwner=tSysTimeFloat()+sg_GetSyncIntervalSelf( this );
        }
        else if ( nextSyncOwner < tSysTimeFloat() &&
                  Owner() != 0 &&
                  sn_Connections[Owner()].bandwidthControl_.Control( nBandwidthControl::Usage_Planning ) > 200 )
        {
            
            RequestSyncOwner();
        }
    }

    return (!Alive());
}


void gCycle::InteractWith(eGameObject *target,REAL,int){
    
}











void gCycle::Die( REAL time )
{
    if ( sn_GetNetState() == nSERVER )
    {
        
        RequestSync( true );
    }

    if( player && Alive() )
    {
        
        player->AnalyzeTiming( -1 );
    }

    if (Alive()) {
#ifndef DEDICATED
        VisualsManager::SpawnDeathBurst(pos.x, pos.y, trailColor_.r, trailColor_.g, trailColor_.b);
        extern REAL sg_modScreenShakeIntensity;
        VisualsManager::TriggerScreenShake(0.5f, sg_modScreenShakeIntensity);

        int killed_slot = (player ? player->pID : 0);
        int killer_slot = killed_slot;
        ePlayerNetID const * constHunter = enemyInfluence.GetEnemy();
        if (constHunter && lastTime - enemyInfluence.GetTime() <= sg_suicideTimeout) {
            killer_slot = constHunter->pID;
        }
        DemoRecorder::Instance().LogKill(se_GameTime(), killed_slot, killer_slot);
#endif
    }

    gCycleMovement::Die( time );


    
    correctPosSmooth = eCoord();
    TransferPositionCorrectionToDistanceCorrection();
    predictPosition_ = pos;

    
    for ( int i = sg_netPlayerWalls.Len()-1; i >= 0; --i )
    {
        gNetPlayerWall * wall = sg_netPlayerWalls(i);
        if ( wall->Cycle() == this && wall->Preliminary() )
        {
            wall->real_CopyIntoGrid(grid);
        }
    }
}

static eLadderLogWriter sg_deathFragWriter("DEATH_FRAG", true);
static eLadderLogWriter sg_deathSuicideWriter("DEATH_SUICIDE", true);
static eLadderLogWriter sg_deathTeamkillWriter("DEATH_TEAMKILL", true);

void gCycle::KillAt( const eCoord& deathPos){
    
    if ( !Vulnerable() )
    {
        MoveSafely( deathPos, lastTime, lastTime );
        return;
    }

    
    ePlayerNetID const * constHunter = enemyInfluence.GetEnemy();
    ePlayerNetID * hunter = Player();

    
    if ( constHunter && constHunter->Object() )
        hunter = constHunter->Object()->Player();

    
    if ( LastTime() - enemyInfluence.GetTime() > sg_suicideTimeout )
        hunter = NULL;

    
    if ( !hunter )
        hunter = Player();


    if (!Alive() || sn_GetNetState()==nCLIENT)
        return;

#ifdef KRAWALL_SERVER_LEAGUE
    if (    hunter           && Player()          &&
            !dynamic_cast<gAIPlayer*>(hunter)     &&
            !dynamic_cast<gAIPlayer*>(Player())             &&
            hunter->IsAuth() && Player()->IsAuth())
        nKrawall::ServerFrag(hunter->GetUserName(), Player()->GetUserName());
#endif

    if (hunter==Player())
    {
        if (hunter)
        {
            sg_deathSuicideWriter << hunter->GetUserName();
            sg_deathSuicideWriter.write();

            if ( score_suicide )
                hunter->AddScore(score_suicide, tOutput(), "$player_lose_suicide" );
            else
            {
                tColoredString hunterName;
                hunterName << *hunter << tColoredString::ColorString(1,1,1);
                sn_ConsoleOut( tOutput( "$player_free_suicide", hunterName ) );
            }
        }
    }
    else{
        if (hunter)
        {
            tOutput lose;
            tOutput win;
            if (Player())
            {
                tColoredString preyName;
                preyName << *Player();
                preyName << tColoredString::ColorString(1,1,1);
                if (Player()->CurrentTeam() != hunter->CurrentTeam()) {
                    sg_deathFragWriter << Player()->GetUserName() << hunter->GetUserName();
                    sg_deathFragWriter.write();

                    win.SetTemplateParameter(3, preyName);
                    win << "$player_win_frag";
                    if ( score_kill != 0 )
                        hunter->AddScore(score_kill, win, lose );
                    else
                    {
                        tColoredString hunterName;
                        hunterName << *hunter << tColoredString::ColorString(1,1,1);
                        sn_ConsoleOut( tOutput( "$player_free_frag", hunterName, preyName ) );
                    }
                }
                else {
                    sg_deathTeamkillWriter << Player()->GetUserName() << hunter->GetUserName();
                    sg_deathTeamkillWriter.write();

                    tColoredString hunterName;
                    hunterName << *hunter << tColoredString::ColorString(1,1,1);
                    sn_ConsoleOut( tOutput( "$player_teamkill", hunterName, preyName ) );
                }
            }
            else
            {
                win << "$player_win_frag_ai";
                hunter->AddScore(score_kill, win, lose);
            }
        }
        
        if (Player())
            Player()->AddScore(score_die,tOutput(),"$player_lose_frag");
    }

    
    this->pos = deathPos;

    Kill();
}

class gJustChecking
{
public:
    static bool justChecking;

    gJustChecking(){ justChecking = false; }
    ~gJustChecking(){ justChecking = true; }
};

bool gJustChecking::justChecking = true;

bool gCycle::EdgeIsDangerous(const eWall* ww, REAL time, REAL a) const{
    gPlayerWall const * w = dynamic_cast< gPlayerWall const * >( ww );
    if ( w )
    {
        if ( !Vulnerable() )
            return false;

        gNetPlayerWall *nw = w->NetWall();

        
        
        
        
        
        if( nw == currentWall || nw == lastWall || ( nw == lastNetWall && Owner() != sn_myNetID ) )
            return false;

        
        
        
        
        
        
        if ( gJustChecking::justChecking && w->CycleMovement() != static_cast< const gCycleMovement * >( this ) && w->Time(a) > time )
        {
            
            
            
            
            gCycle const * otherPlayer = w->Cycle();
            if ( !otherPlayer || 
                    otherPlayer->Team() != this->Team() || 
                    !otherPlayer->currentWall || w == otherPlayer->currentWall->Wall() 
               )
                return false;
        }
    }

    return gCycleMovement::EdgeIsDangerous( ww, time, a );
}


static void sg_KillFutureWall( gCycle * cycle, gNetPlayerWall * wall )
{
    if ( cycle && wall && wall->Cycle() == cycle && wall->Pos(1) > cycle->GetDistance() )
    {
        wall->BlowHole( cycle->GetDistance(), wall->Pos(1) + 100, 0 );
    }
}


static void sg_KillFutureWalls( gCycle * cycle )
{
#ifdef DEBUG_X
    con << "Removing future walls of the cylce that just got killed mercilessly.\n";
#endif

    
    if ( sn_GetNetState() != nCLIENT )
    {
        int i;
        for ( i = sg_netPlayerWalls.Len()-1; i >= 0; --i )
            sg_KillFutureWall( cycle, sg_netPlayerWalls(i) );

        for ( i = sg_netPlayerWallsGridded.Len()-1; i >= 0; --i )
            sg_KillFutureWall( cycle, sg_netPlayerWallsGridded(i) );
    }
}

static void sg_HoleScore( gCycle & cycle )
{
    cycle.Player()->AddScore( score_hole, tOutput("$player_win_hole"), tOutput("$player_lose_hole") );
}

static eLadderLogWriter sg_sacrificeWriter("SACRIFICE", true);

void gCycle::PassEdge(const eWall *ww,REAL time,REAL a,int){
    {
        
        gJustChecking thisIsSerious;

        if (!EdgeIsDangerous(ww,time,a) || !Alive() )
        {
            
            if ( ( !currentWall || ww != currentWall->Wall() ) && ( !lastWall || ww != lastWall->Wall() ) )
                RequestSyncAll();
            
            
            gPlayerWall const * w = dynamic_cast< gPlayerWall const * >( ww );
            if ( Alive() && w && score_hole )
            {
                gExplosion * explosion = w->Holer( a, time );
                if ( explosion )
                {
                    gCycle * holer = explosion->GetOwner();
                    if ( holer && holer != this && holer->Player() &&
                         Player() &&
                         w->Cycle() && w->Cycle()->Player() &&
                         holer->Player()->CurrentTeam() == Player()->CurrentTeam() &&       
                         w->Cycle()->Player()->CurrentTeam() != Player()->CurrentTeam()  
                        )
                    {
                        
                        if ( explosion->AccountForHole() )
                        {
                            sg_sacrificeWriter << Player()->GetUserName() << holer->Player()->GetUserName() << w->Cycle()->Player()->GetUserName();
                            sg_sacrificeWriter.write();
                            if ( score_hole > 0 )
                            {
                                
                                sg_HoleScore( *holer );
                            }
                            else
                            {
                                
                                sg_HoleScore( *this );

                            }
                       }
                    }
                }
            }

            return;
        }

#ifdef DEBUG
        if (!EdgeIsDangerous(ww,time,a) || !Alive() )
            return;
#endif
    }

    
    RequestSyncOwner();

#ifdef DEBUG_X
    
    tJUST_CONTROLLED_PTR<gCycleExtrapolator> keepOther( extrapolator_ );

    ResetExtrapolator();

    
    REAL dt = 1;
    for ( int i = 9; i>= 0; --i )
    {
        Extrapolate( dt );
    }

    extrapolator_ = keepOther;
#endif

    eCoord collPos = ww->Point( a );

    const gPlayerWall *w = dynamic_cast<const gPlayerWall*>(ww);

    enemyInfluence.AddWall( ww, collPos, 0, this );

    if (w)
    {
        gCycle *otherPlayer=w->Cycle();

        REAL otherTime = w->Time(a);
        if(otherPlayer && time < otherTime*(1-EPS) )
        {
            
            otherPlayer->RequestSyncOwner();

            
            REAL wallDist = w->Pos(a);
            
            REAL cycleDist = w->Cycle()->distance;
            
            if ( wallDist > cycleDist * (1 + EPS ) )
            {
                static bool fix = false;
                
                if ( fix && lastTime > se_GameTime() - 2 * Lag() - GetMaxLazyLag() )
                    throw gCycleStop();
                else
                    return;
            }

            
            if ( otherPlayer->Vulnerable() )
            {
                static bool tryToSaveFutureWallOwner = true;
                bool saved = false;

                if ( tryToSaveFutureWallOwner && otherPlayer->currentWall && w == otherPlayer->currentWall->Wall() )
                {
                    
                    
                    
                    REAL d = otherPlayer->GetDistanceSinceLastTurn() * .001;
                    if ( d < .01 )
                        d = .01;
                    REAL maxd = eCoord::F( otherPlayer->dirDrive, collPos - otherPlayer->GetLastTurnPos() ) * .5/otherPlayer->dirDrive.NormSquared();
                    if ( d > maxd )
                        d = maxd;
                    if ( d > 0 )
                    {
                        saved = true;

                        
                        otherPlayer->MoveSafely( collPos-otherPlayer->dirDrive*d, otherPlayer->LastTime(), otherTime - d/otherPlayer->Speed() );
                        otherPlayer->currentWall->Update( otherPlayer->lastTime, otherPlayer->pos );
                        otherPlayer->dropWallRequested_ = false;

                        
                        dropWallRequested_ = true;
                    }
                }

                
                
                
                if ( !saved && verletSpeed_ >= 0 && this->ThisWallsLength() > 0 )
                {
                    REAL dt = otherTime - time;

                    
                    
                    REAL wallTimeLeft = this->ThisWallsLength()/verletSpeed_ - dt;

                    if ( wallTimeLeft < 0 )
                    {
                        
                        return;
                    }

                    
                    REAL max, effectiveness;
                    sg_RubberValues( otherPlayer->Player(), otherPlayer->verletSpeed_, max, effectiveness );
                    if ( effectiveness > 0 )
                    {
                        REAL rubberToEat = wallTimeLeft * otherPlayer->Speed()/effectiveness;

                        otherPlayer->rubber += rubberToEat;
                        if ( otherPlayer->rubber > max )
                            otherPlayer->rubber = max; 
                        else
                            saved = true;              
                    }
                }

                if ( !saved && sn_GetNetState() != nCLIENT )
                {
                    
                    if ( currentWall )
                        otherPlayer->enemyInfluence.AddWall( currentWall->Wall(), lastTime, 0, otherPlayer );
                    otherPlayer->distance = wallDist;
                    otherPlayer->DropWall();
                    otherPlayer->KillAt( collPos );

                    
                    sg_KillFutureWalls( otherPlayer );
                }
            }
        }
        else 
        {
            
            throw gCycleDeath( collPos );

            
            
        }
    }
    else
    {
        if (bool(player) && sn_GetNetState()!=nCLIENT && Alive() )
        {
            throw gCycleDeath( collPos );
        }

    }
}

REAL gCycle::PathfindingModifier( const eWall *w ) const
{
    if (!w)
        return 1;
    if (dynamic_cast<const gPlayerWall*>(w))
        return .9f;
    else
        return 1;
}


bool gCycle::Act(uActionPlayer *Act, REAL x){
    
    if (se_mainGameTimer && ( se_mainGameTimer->speed <= 0 || se_mainGameTimer->Time() < -1 ) )
        return false;

    if (!Alive() && sn_GetNetState()==nSERVER)
        RequestSync(false);

    if(se_turnLeft==*Act && x>.5){
        
        extern bool sg_modAnti360LockEnabled;
        extern REAL sn_anti360Window;
        if (sg_modAnti360LockEnabled && Alive()) {
            REAL now = tSysTimeFloat();
            
            if (leftTurnCount >= 3 && (now - leftTurnTimes[0]) <= sn_anti360Window) {
                
                return true;
            }
            
            rightTurnCount = 0; 
            if (leftTurnCount < 3) {
                leftTurnTimes[leftTurnCount] = now;
                leftTurnCount++;
            } else {
                
                leftTurnTimes[0] = leftTurnTimes[1];
                leftTurnTimes[1] = leftTurnTimes[2];
                leftTurnTimes[2] = now;
            }
        }
        Turn(-1);
        return true;
    }
    else if(se_turnRight==*Act && x>.5){
        
        extern bool sg_modAnti360LockEnabled;
        extern REAL sn_anti360Window;
        if (sg_modAnti360LockEnabled && Alive()) {
            REAL now = tSysTimeFloat();
            
            if (rightTurnCount >= 3 && (now - rightTurnTimes[0]) <= sn_anti360Window) {
                
                return true;
            }
            
            leftTurnCount = 0; 
            if (rightTurnCount < 3) {
                rightTurnTimes[rightTurnCount] = now;
                rightTurnCount++;
            } else {
                
                rightTurnTimes[0] = rightTurnTimes[1];
                rightTurnTimes[1] = rightTurnTimes[2];
                rightTurnTimes[2] = now;
            }
        }
        Turn(1);
        return true;
    }
    else if(s_brake==*Act){
        
        unsigned short newBraking=(x>0);
        if ( braking != newBraking )
        {
            AccelerationDiscontinuity();
            braking = newBraking;
            AddDestination();
        }
        return true;
    }
    else if(s_brakeToggle==*Act){
        if ( x > 0 )
        {
            AccelerationDiscontinuity();
            braking = !braking;
            AddDestination();
        }
        return true;
    }
    
    else if(Act->internalName == "CYCLE_PERFECT_LEFT") {
        return true;
    }
    else if(Act->internalName == "CYCLE_PERFECT_RIGHT") {
        return true;
    }
    
    else if(Act->internalName == "CYCLE_AUTOESCAPE_LEFT") {
        return true;
    }
    else if(Act->internalName == "CYCLE_AUTOESCAPE_RIGHT") {
        return true;
    }
    return false;
}


static nVersionFeature sg_SyncsAreUsed( 5 );


static eCoord const * sg_fakeDirDrive = NULL;
class gFakeDirDriveSetter
{
public:
    gFakeDirDriveSetter( eCoord const & dir )
            : lastFakeDir_( sg_fakeDirDrive )
    {
        sg_fakeDirDrive = &dir;
    }

    ~gFakeDirDriveSetter()
    {
        sg_fakeDirDrive = lastFakeDir_;
    }
private:
    eCoord const * lastFakeDir_;
};


REAL sn_anti360Window = 1.0f;
static tConfItem<REAL> sn_anti360WindowConf("ANTI_360_WINDOW", sn_anti360Window);

bool gCycle::DoTurn(int d)
{
#ifdef DELAYEDTURN_DEBUG
    REAL delay = tSysTimeFloat() - sg_turnReceivedTime;
    if ( delay > EPS && sn_GetNetState() == nSERVER && Owner() != 0 )
    {
        con << "Delayed turn execution! " << turns << "\n";
    }
#endif

#ifdef GNUPLOT_DEBUG
    if ( sg_gnuplotDebug && Player() )
    {
        std::ofstream f( Player()->GetUserName() + "_turn", std::ios::app );
        f << pos.x << " " << pos.y << "\n";
    }
#endif

    if (d >  1) d =  1;
    if (d < -1) d = -1;

    

    if (Alive()){
        if ( turning )
            turning->Reset();

        clientside_action();

        if ( gCycleMovement::DoTurn( d ) )
        {
#ifndef DEDICATED
            DemoRecorder::Instance().LogTurn(se_GameTime(), (player ? player->pID : 0), d);
#endif
            sg_ArchiveCoord( pos, 1 );

            skewDot+=4*d;

            if (sn_GetNetState() == nCLIENT && Owner() == ::sn_myNetID)
                AddDestination();

            if (sn_GetNetState()!=nCLIENT)
            {
                RequestSync();
            }

            
            
            {
                FindCurrentFace();
                REAL factor = -16;
                eCoord dirDriveFake = dirDrive * factor;
                eCoord lastDirDriveBack = lastDirDrive;
                lastDirDrive = lastDirDrive * factor;
                gFakeDirDriveSetter fakeSetter( dirDriveFake );
                DropWall();
                lastDirDrive = lastDirDriveBack;
            }

            return true;
        }
    }

    return false;
}

void gCycle::DropWall( bool buildNew )
{
    
    tJUST_CONTROLLED_PTR< gCycle > keep( this->GetRefcount()>0 ? this : 0 );

    
    if ( lastWall && lastNetWall && lastWall->Time(.5) > lastNetWall->Time(0) )
        lastNetWall = 0;

    
    if(currentWall)
    {
        lastWall=currentWall;
        currentWall->Update(lastTime,pos);
        currentWall->CopyIntoGrid( grid );
        currentWall=NULL;
    }

    if ( buildNew && lastTime >= spawnTime_ + sg_cycleWallTime )
        currentWall=new gNetPlayerWall(this,pos,dirDrive,lastTime,distance);

    
    
    eCoord dirDriveBack = dirDrive;
    if ( sg_fakeDirDrive )
        dirDrive = *sg_fakeDirDrive;

    if ( grid )
    {
        for(int i=grid->GameObjects().Len()-1;i>=0;i--)
        {
            eGameObject * c = grid->GameObjects()(i);
            if (c->CurrentFace() && !c->CurrentFace()->IsInGrid() )
                c->FindCurrentFace();
        }
    }
    dirDrive = dirDriveBack;

    
    dropWallRequested_ = false;
}

void gCycle::Kill(){
    
    tJUST_CONTROLLED_PTR< gCycle > keep( this->GetRefcount()>0 ? this : 0 );

    if (sn_GetNetState()!=nCLIENT){
        RequestSync(true);
        if (Alive() && grid && GOID() >= 0 ){
            Die( lastTime );
            tNEW(gExplosion)(grid, pos,lastTime, color_, this );
            

            if ( currentWall )
            {
                
                
                
                

                
                if ( currentWall->Pos(1) > distance || currentWall->Time(1) > lastTime )
                    currentWall->Update( lastTime, pos );

                
                currentWall->CopyIntoGrid( 0 );

                currentWall = NULL;
            }
        }
    }
    
    
    
    
}



static rFileTexture cycle_shad(rTextureGroups::TEX_FLOOR,"textures/shadow.png",0,0,true);


#define ENABLE_OLD_LAG_O_METER

REAL sg_laggometerScale=1;
static tSettingItem< REAL > sg_laggometerScaleConf( "LAG_O_METER_SCALE", sg_laggometerScale );
REAL sg_laggometerThreshold=.5;
static tSettingItem< REAL > sg_laggometerThresholdConf( "LAG_O_METER_THRESHOLD", sg_laggometerThreshold );
REAL sg_laggometerBlend=.5;
static tSettingItem< REAL > sg_laggometerBlendConf( "LAG_O_METER_BLEND", sg_laggometerBlend );
#ifdef ENABLE_OLD_LAG_O_METER
bool sg_laggometerUseOld=false;
static tSettingItem< bool > sg_laggometerUseOldConf( "LAG_O_METER_USE_OLD", sg_laggometerUseOld );
#endif
bool sg_axesIndicator=false;

int sg_blinkFrequency=10;
static tSettingItem< int > sg_blinkFrequencyConf( "CYCLE_BLINK_FREQUENCY", sg_blinkFrequency );



#ifndef DEDICATED

namespace gLaggometer {
class Colour {
public:
    REAL cp[3];
    Colour(REAL r, REAL g, REAL b) {
        cp[0]=r;
        cp[1]=g;
        cp[2]=b;
    }
    Colour(ePlayerNetID* player) {
        if ( player )
        {
            player->Color(cp[0], cp[1], cp[2]);
        }
        else
        {
            cp[0]=cp[1]=cp[2]=1;
        }
    }
    void blend(REAL factor, const Colour& target) {
        for (int i=0; i<3; i++) {
            cp[i] = (1 - factor) * cp[i] + factor * target.cp[i];
        }
    }
    void toGl() const { glColor3f(cp[0], cp[1], cp[2]); }

    static const Colour white;
    static const Colour black;
};

const Colour Colour::white(1,1,1);
const Colour Colour::black(0,0,0);

class DirectionTransformer {
private:
    eGrid* grid;
    eCoord factor;
public:
    DirectionTransformer(eGrid* theGrid) : grid(theGrid), factor(theGrid->GetDirection(0).Conj()) { }
    eCoord get(int i) {
        return grid->GetDirection(i).Turn(factor);

    }
    int ahead() { return 2 * (grid->WindingNumber()); }
};

class LagOMeterRenderer {
private:
    DirectionTransformer directions;
    REAL delay;
    Colour color;
protected:
    bool drawTriangle(eCoord loc, int winding, REAL lag, int inc);
public:
    LagOMeterRenderer(gCycle* cycle) :
            directions(cycle->Grid()),
            delay(cycle->GetTurnDelay()),
            color(cycle->Player())
    {
        color.blend(sg_laggometerBlend, Colour::white);
    }
    void render(REAL lag);
};


bool LagOMeterRenderer::drawTriangle(eCoord loc, int winding, REAL lag, int inc) {
    eCoord outer = loc + directions.get(winding) * lag;
    if (outer.y * inc > 0.01f) {
        eCoord oldOuter = loc + directions.get(winding - inc) * lag;
        eCoord d = outer - oldOuter;
        outer = oldOuter + d * (-oldOuter.y / d.y);
        glVertex2f(outer.x, outer.y);
        return true;
    } else {
        glVertex2f(outer.x, outer.y);
        if (lag > delay) {
            if (drawTriangle(loc + directions.get(winding + inc) * delay, winding + inc, lag - delay, inc)) return true;
        } else {
            outer = loc + directions.get(winding + inc) * lag;
            glVertex2f(outer.x, outer.y);
        }
        glVertex2f(loc.x, loc.y);
        return false;
    }
}

void LagOMeterRenderer::render(REAL lag) {
    color.toGl();
    BeginLineStrip();
    drawTriangle(eCoord(0,0), directions.ahead(), lag, 1);
    RenderEnd();

    BeginLineStrip();
    drawTriangle(eCoord(0,0), directions.ahead(), lag, -1);
    RenderEnd();
}


class AxesIndicator {
private:
    DirectionTransformer directions;
    Colour color;
public:
    AxesIndicator(gCycle* cycle) :
            directions(cycle->Grid()),
            color(cycle->Player())
    {
        color.blend(.5f, Colour::white);
    }
    void line(int i) {
        eCoord midle = directions.get(directions.ahead() + i) * .1f;
        eCoord inner = midle * .5f;
        eCoord outer = midle + directions.get(directions.ahead() + 2 * i) * .05f;

        BeginLineStrip();
        
        color.toGl();
        glVertex2f(inner.x, inner.y);


        glVertex2f(midle.x, midle.y);

        Colour::black.toGl();
        glVertex2f(outer.x, outer.y);
        RenderEnd();
    }
    void render() {
        
        glShadeModel(GL_SMOOTH);
        line(-1);
        line(0);
        line(1);
    }
};

}

static REAL mp_eWall_stretch=4;
static tSettingItem<REAL> mpws
("MOVIEPACK_WALL_STRETCH",mp_eWall_stretch);

static rFileTexture dir_eWall(rTextureGroups::TEX_WALL,"textures/dir_wall.png",1,0,1);
static rFileTexture dir_eWall_moviepack(rTextureGroups::TEX_WALL,"moviepack/dir_wall.png",1,0,1);

static void dir_eWall_select()
{
    if (sg_MoviePack()){
        TexMatrix();
        IdentityMatrix();
        ScaleMatrix(1/mp_eWall_stretch,1,1);
        dir_eWall_moviepack.Select();
    }
    else
    {
        dir_eWall.Select();
    }
}

gCycleWallsDisplayListManager::gCycleWallsDisplayListManager()
    : wallList_(0)
    , wallsWithDisplayList_(0)
    , wallsWithDisplayListMinDistance_(0)
    , wallsInDisplayList_(0)
{
}

gCycleWallsDisplayListManager::~gCycleWallsDisplayListManager()
{
    while(wallList_)
        wallList_->Remove();
    while(wallsWithDisplayList_)
        wallsWithDisplayList_->Remove();
}


bool gCycleWallsDisplayListManager::CannotHaveList( REAL distance, gCycle const * cycle )
{
    extern bool sg_corpseTimerOverride;
    extern float sg_corpseTimerDuration;
    REAL totalDelay = gCycle::WallsStayUpDelay();
    if (totalDelay < 0.0f || sg_corpseTimerOverride) {
        totalDelay = sg_corpseTimerDuration;
    }
    return
            ( !cycle->Alive() && totalDelay >= 0.0f && se_GameTime()-cycle->DeathTime()-totalDelay > 0 ) 

            ||

            ( cycle->ThisWallsLength() > 0 && cycle->GetDistance() - cycle->ThisWallsLength() > distance );
}

void gCycleWallsDisplayListManager::RenderAllWithDisplayList( eCamera const * camera, gCycle * cycle )
{
    dir_eWall_select();

    glDisable(GL_CULL_FACE);
    
    gNetPlayerWall * run = 0;
    

    int wallsWithPossibleDisplayList = 0;
    run = wallList_;
    while( run )
    {
        gNetPlayerWall * next = run->Next();
        if ( run->CanHaveDisplayList() )
        {
            wallsWithPossibleDisplayList++;
        }
        else
        {
            
            if ( cycle->ThisWallsLength() > 0 && cycle->GetDistance() - cycle->MaxWallsLength() > run->EndPos() )
                
            {
                run->Remove();
            }
        }
        run = next;
    }

    
    bool tailExpired=false;
    if ( CannotHaveList( wallsWithDisplayListMinDistance_, cycle ) )
    {
        tailExpired=true;
        displayList_.Clear(0);
    }
    
    else if ( wallsWithPossibleDisplayList >= 3 ||
         wallsWithPossibleDisplayList * 5 > wallsInDisplayList_ )
    {
        
        displayList_.Clear(0);
    }

    
    if ( displayList_.Call() )
    {
        return;
    }

    
    run = wallsWithDisplayList_;
    while( run )
    {
        gNetPlayerWall * next = run->Next();
        if ( !run->CanHaveDisplayList() || ( tailExpired && wallsWithDisplayListMinDistance_ >= run->BegPos() ) )
        {
            run->Insert( wallList_ );
        }
        run = next;
    }

    if ( wallsWithPossibleDisplayList > 0 )
    {
        run = wallList_;
        while( run )
        {
            gNetPlayerWall * next = run->Next();
            if ( run->CanHaveDisplayList() )
            {
                run->Insert( wallsWithDisplayList_ );
            
                
                run->ClearDisplayList(0, -1);
            }
        
            run = next;
        }
    }

    if ( !wallsWithDisplayList_ )
    {
        return;
    }

    
    rDisplayListFiller filler( displayList_ );

    if ( rDisplayList::IsRecording() )
    {
        wallsWithDisplayListMinDistance_ = 1E+30;
        wallsInDisplayList_ = 0;

        
        run = wallsWithDisplayList_;
        while( run )
        {
            gNetPlayerWall * next = run->Next();
            if ( run->BegPos() < wallsWithDisplayListMinDistance_ )
            {
                wallsWithDisplayListMinDistance_ = run->BegPos();
            }
            wallsInDisplayList_++;
            run = next;
        }
    }

    
    RenderAll( camera, cycle, wallsWithDisplayList_ );
}

void gCycleWallsDisplayListManager::RenderAll( eCamera const * camera, gCycle * cycle, gNetPlayerWall * list )
{
    if( !list )
    {
        return;
    }

    
    sr_DepthOffset(true);
    if ( rTextureGroups::TextureMode[rTextureGroups::TEX_WALL] != 0 )
        glDisable(GL_TEXTURE_2D);
    
    gNetPlayerWall * run = list;
    while( run )
    {
        gNetPlayerWall * next = run->Next();
        run->RenderList( true, gNetPlayerWall::gWallRenderMode_Lines );
        run = next;
    }

    RenderEnd();
    sr_DepthOffset(false);
    if ( rTextureGroups::TextureMode[rTextureGroups::TEX_WALL] != 0 )
        glEnable(GL_TEXTURE_2D);
    
    run = list;
    while( run )
    {
        gNetPlayerWall * next = run->Next();
        run->RenderList( true, gNetPlayerWall::gWallRenderMode_Quads );
        run = next;
    }

    RenderEnd();
}

void gCycleWallsDisplayListManager::RenderAll( eCamera const * camera, gCycle * cycle )
{
    extern bool cfg_EnableInstancing;
    if (cfg_EnableInstancing) {
        dir_eWall_select();
        if ( rTextureGroups::TextureMode[rTextureGroups::TEX_WALL] != 0 )
            glEnable(GL_TEXTURE_2D);
        else
            glDisable(GL_TEXTURE_2D);

        
        RenderAll( camera, cycle, wallList_ );
        RenderAll( camera, cycle, wallsWithDisplayList_ );
        
        
        if (!g_collectedTrails.empty()) {
            float viewMatrix[16];
            float projectionMatrix[16];
            glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);
            glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
            
            g_trailRenderer.RenderTrails(g_collectedTrails, viewMatrix, projectionMatrix);
            g_collectedTrails.clear();
        }
        return;
    }

    
    RenderAllWithDisplayList( camera, cycle );

    
    RenderAll( camera, cycle, wallList_ );
}

void gCycle::UpdateTexturesColor() {
#ifndef DEDICATED
    
    if (lastTexR_ == color_.r && lastTexG_ == color_.g && lastTexB_ == color_.b) {
        return;
    }

    double now = tSysTimeFloat();
    if (now - lastTexUpdateTime_ < 0.033) {
        return; 
    }
    lastTexUpdateTime_ = now;

    lastTexR_ = color_.r;
    lastTexG_ = color_.g;
    lastTexB_ = color_.b;

    int preference = mp ? 1 : 0;

    if (customTexture) {
        delete customTexture;
        customTexture = NULL;
        rSurface * surface = gCycleVisuals::LoadTextureSafe2( gCycleVisuals::SLOT_CUSTOM, preference );
        if ( !surface )
            surface = gCycleVisuals::LoadTextureSafe2( gCycleVisuals::SLOT_CUSTOM, 1-preference );
        if ( surface )
            customTexture = tNEW( gTextureCycle )( *surface, color_, 0, 0, false );
    }

    if (bodyTex) {
        delete bodyTex;
        bodyTex = NULL;
        rSurface * surface = gCycleVisuals::LoadTextureSafe2( gCycleVisuals::SLOT_BODY, preference );
        if ( !surface )
            surface = gCycleVisuals::LoadTextureSafe2( gCycleVisuals::SLOT_BODY, 1-preference );
        if ( surface )
            bodyTex = tNEW( gTextureCycle )( *surface, color_, 0, 0, false );
    }

    if (wheelTex) {
        delete wheelTex;
        wheelTex = NULL;
        rSurface * surface = gCycleVisuals::LoadTextureSafe2( gCycleVisuals::SLOT_WHEEL, preference );
        if ( !surface )
            surface = gCycleVisuals::LoadTextureSafe2( gCycleVisuals::SLOT_WHEEL, 1-preference );
        if ( surface )
            wheelTex = tNEW( gTextureCycle )( *surface, color_, 0, 0, true );
    }
#endif
}

void gCycle::Render(const eCamera *cam){
#ifndef DEDICATED
    UpdateTexturesColor();
#endif
    

    
    bool blinking = false;
    if ( lastTime > spawnTime_ && !Vulnerable() )
    {
        double time = tSysTimeFloat();
        double wrap = time - floor(time);
        int pulse = int ( 2 * wrap * sg_blinkFrequency );
        blinking = pulse & 1;
    }

#ifdef USE_HEADLIGHT
#ifdef LINUX
    typedef void (*glProgramStringARB_Func)(GLenum, GLenum, GLsizei, const void*);
    glProgramStringARB_Func glProgramStringARB_ptr = 0;

    typedef void (*glProgramLocalParameter4fARB_Func)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    glProgramLocalParameter4fARB_Func glProgramLocalParameter4fARB_ptr = 0;

    glProgramStringARB_ptr = (glProgramStringARB_Func) SDL_GL_GetProcAddress("glProgramStringARB");
    glProgramLocalParameter4fARB_ptr = (glProgramLocalParameter4fARB_Func) SDL_GL_GetProcAddress("glProgramLocalParameter4fARB");
#endif
#endif    
    if (!std::isfinite(z) || !std::isfinite(pos.x) ||!std::isfinite(pos.y)||!std::isfinite(dir.x)||!std::isfinite(dir.y)
            || !std::isfinite(skew))
        st_Breakpoint();
    if (Alive()){
        

#ifdef DEBUG
        
#endif

        GLfloat color[4]={1,1,1,1};
        static GLfloat lposa[4] = { 320, 240, 200,0};
        static GLfloat lposb[4] = { -240, -100, 200,0};
        static GLfloat lighta[4] = { 1, .7, .7, 1 };
        static GLfloat lightb[4] = { .7, .7, 1, 1 };

        glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,color);
        glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,color);

        glLightfv(GL_LIGHT0, GL_DIFFUSE, lighta);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lighta);
        glLightfv(GL_LIGHT0, GL_POSITION, lposa);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, lightb);
        glLightfv(GL_LIGHT1, GL_SPECULAR, lightb);
        glLightfv(GL_LIGHT1, GL_POSITION, lposb);


        ModelMatrix();
        glPushMatrix();
        eCoord p = PredictPosition();
        glTranslatef(p.x,p.y,0);
        glScalef(.5f,.5f,.5f);


        eCoord ske(1,skew);
        ske=ske*(1/sqrt(ske.NormSquared()));

        GLfloat m[4][4]={{dir.x,dir.y,0,0},
                         {-dir.y,dir.x,0,0},
                         {0,0,1,0},
                         {0,0,0,1}};
        glMultMatrixf(&m[0][0]);

        glPushMatrix();
        
        if (!mp)
            glTranslatef(-1.5,0,0);

        glPushMatrix();

        GLfloat sk[4][4]={{1,0,0,0},
                          {0,ske.x,ske.y,0},
                          {0,-ske.y,ske.x,0},
                          {0,0,0,1}};

        glMultMatrixf(&sk[0][0]);


        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHTING);



        TexMatrix();
        IdentityMatrix();

        if (mp){

            ModelMatrix();
            if ( !blinking )
            {
                glPushMatrix();
                customTexture->Select();
                glColor3f(1,1,1);
                customModel->Render();
                glPopMatrix();
            }

            glPopMatrix();
            glTranslatef(-1.5,0,0);
        }
        else{
            glEnable(GL_TEXTURE_2D);

            ModelMatrix();

            if ( !blinking )
            {
                bodyTex->Select();
                body->Render();

                wheelTex->Select();
                
                glPushMatrix();
                glTranslatef(0,0,.73);
                
                GLfloat mr[4][4]={{rotationRearWheel.x,0,rotationRearWheel.y,0},
                                  {0,1,0,0},
                                  {-rotationRearWheel.y,0,rotationRearWheel.x,0},
                                  {0,0,0,1}};
                
                
                glMultMatrixf(&mr[0][0]);
                
                rear->Render();
                glPopMatrix();

                glPushMatrix();
                glTranslatef(1.84,0,.43);

                GLfloat mf[4][4]={{rotationFrontWheel.x,0,rotationFrontWheel.y,0},
                                  {0,1,0,0},
                                  {-rotationFrontWheel.y,0,rotationFrontWheel.x,0},
                                  {0,0,0,1}};
                
                glMultMatrixf(&mf[0][0]);

                front->Render();
                glPopMatrix();
            }
            
            glPopMatrix();
        }


        
        
        ModelMatrix();

        

        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHT1);
        glDisable(GL_LIGHTING);

        
        glDisable(GL_TEXTURE_2D);
        glColor3f(1,1,1);

        {
            bool renderPyramid = false;
            gRealColor colorPyramid;
            REAL alpha = 1;
            const REAL timeout = .5f;

            if ( bool(player) )
            {
                if ( player->IsChatting() )
                {
                    renderPyramid = true;
                    colorPyramid.b = 0.0f;
                }
                else if ( !player->IsActive() )
                {
                    renderPyramid = true;
                    colorPyramid.b = 0.0f;
                    colorPyramid.g = 0.0f;
                }
                else if ( cam && cam->Center() == this && se_GameTime() < timeout && player->CurrentTeam() && player->CurrentTeam()->NumPlayers() > 1 )
                {
                    renderPyramid = true;
                    alpha = timeout - se_GameTime();
                }
            }

            if ( renderPyramid )
            {
                GLfloat s=sin(lastTime);
                GLfloat c=cos(lastTime);

                GLfloat m[4][4]={{c,s,0,0},
                                 {-s,c,0,0},
                                 {0,0,1,0},
                                 {0,0,1,1}};

                glPushMatrix();

                glMultMatrixf(&m[0][0]);
                glScalef(.5,.5,.5);


                BeginTriangles();

                glColor4f( colorPyramid.r,colorPyramid.g,colorPyramid.b, alpha );
                glVertex3f(0,0,3);
                glVertex3f(0,1,4.5);
                glVertex3f(0,-1,4.5);

                glColor4f( colorPyramid.r * .7f,colorPyramid.g * .7f,colorPyramid.b * .7f, alpha );
                glVertex3f(0,0,3);
                glVertex3f(1,0,4.5);
                glVertex3f(-1,0,4.5);

                RenderEnd();

                glPopMatrix();
            }
        }

#ifdef USE_HEADLIGHT
        
        if(headlights) {
            if(!cycleprograminited) { 
                const char *program =
                    "!!ARBfp1.0\
                    \
                    PARAM normal = program.local[0];\
                    ATTRIB texcoord = fragment.texcoord;\
                    TEMP final, diffuse, distance;\
                    \
                    DP3 distance, texcoord, texcoord;\
                    RSQ diffuse, distance.w;\
                    RCP distance, distance.w;\
                    MUL diffuse, texcoord, diffuse;\
                    DP3 diffuse, diffuse, normal;\
                    MUL final, diffuse, distance;\
                    MOV result.color.w, fragment.color;\
                    MUL result.color.xyz, fragment.color, final;\
                    \
                    END";
#ifdef LINUX
                glProgramStringARB_ptr(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(program), program);
#else
                glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(program), program);
#endif
                cycleprograminited = true;
            }
#ifdef LINUX
            glProgramLocalParameter4fARB_ptr(GL_FRAGMENT_PROGRAM_ARB, 0, 0, 0, verletSpeed_ * verletSpeed_, 0);
#else			
            glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, 0, 0, verletSpeed_ * verletSpeed_, 0);
#endif
            glPushAttrib(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
            glEnable(GL_FRAGMENT_PROGRAM_ARB); 

            const unsigned sensors = 32; 
            const double mul = 0.25 * M_PI / sensors;
            const double add = -0.125 * M_PI;

            double size = gArena::SizeMultiplier() * 500 * M_SQRT2; 
            GLfloat array[sensors+2][5];

            array[0][0] = 0;
            array[0][1] = 0;
            array[0][2] = p.x;
            array[0][3] = p.y;
            array[0][4] = 0.125;

            for(unsigned i=0; i<=sensors; i++) {
                gSensor sensor(this, p, dir.Turn(cos(i * mul + add), sin(i * mul + add)));
                sensor.detect(size);
                array[i][5] = sensor.before_hit.x - p.x;
                array[i][6] = sensor.before_hit.y - p.y;
                array[i][7] = sensor.before_hit.x;
                array[i][8] = sensor.before_hit.y;
                array[i][9] = 0.125;
            }

            glPushMatrix();
            glLoadIdentity();

            glMatrixMode(GL_TEXTURE);
            glPushMatrix();
            glTranslatef(0, 0, 1);

            glBlendFunc(GL_ONE, GL_ONE);
            glDepthMask(GL_FALSE);

            glColor3fv(reinterpret_cast<GLfloat *>(&color_)); 
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glInterleavedArrays(GL_T2F_V3F, 0, array);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sensors+2);

            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            glDisable(GL_FRAGMENT_PROGRAM_ARB);

            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);

            glPopMatrix();
            glPopAttrib();
        }
#endif 
        
        RenderName( cam );


        

        sr_DepthOffset(true);


        REAL h=0;

        glEnable(GL_CULL_FACE);

        if(!blinking && sr_floorDetail>rFLOOR_GRID && rTextureGroups::TextureMode[rTextureGroups::TEX_FLOOR]>0 && sr_alphaBlend){
            glColor3f(0,0,0);
            cycle_shad.Select();
            BeginQuads();
            glTexCoord2f(0,1);
            glVertex3f(-.6,.4,h);

            glTexCoord2f(1,1);
            glVertex3f(-.6,-.4,h);

            glTexCoord2f(1,0);
            glVertex3f(2.1,-.4,h);

            glTexCoord2f(0,0);
            glVertex3f(2.1,.4,h);

            RenderEnd();
        }

        glDisable(GL_CULL_FACE);

        


        REAL f=verletSpeed_;

        REAL l=Lag();

        extern bool g_CustomHitbox;
        if (g_CustomHitbox) {
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glLineWidth(1.5f);
            glColor3f(0.0f, 1.0f, 0.5f);
            float xmin = -1.5f;
            float xmax = 2.2f;
            float ymin = -0.4f;
            float ymax = 0.4f;
            float zmin = 0.0f;
            float zmax = 1.4f;
            glBegin(GL_LINES);
            glVertex3f(xmin, ymin, zmin); glVertex3f(xmax, ymin, zmin);
            glVertex3f(xmax, ymin, zmin); glVertex3f(xmax, ymax, zmin);
            glVertex3f(xmax, ymax, zmin); glVertex3f(xmin, ymax, zmin);
            glVertex3f(xmin, ymax, zmin); glVertex3f(xmin, ymin, zmin);
            glVertex3f(xmin, ymin, zmax); glVertex3f(xmax, ymin, zmax);
            glVertex3f(xmax, ymin, zmax); glVertex3f(xmax, ymax, zmax);
            glVertex3f(xmax, ymax, zmax); glVertex3f(xmin, ymax, zmax);
            glVertex3f(xmin, ymax, zmax); glVertex3f(xmin, ymin, zmax);
            glVertex3f(xmin, ymin, zmin); glVertex3f(xmin, ymin, zmax);
            glVertex3f(xmax, ymin, zmin); glVertex3f(xmax, ymin, zmax);
            glVertex3f(xmax, ymax, zmin); glVertex3f(xmax, ymax, zmax);
            glVertex3f(xmin, ymax, zmin); glVertex3f(xmin, ymax, zmax);
            glEnd();
            glLineWidth(1.0f);
            glEnable(GL_LIGHTING);
            glEnable(GL_TEXTURE_2D);
        }

        glPopMatrix();

        h=cam->CameraZ()*.005+.03;

#ifdef ENABLE_OLD_LAG_O_METER
        extern bool sg_noclipCinematic;
        if(sg_laggometerUseOld) {
            if (sn_GetNetState() != nSTANDALONE && sr_laggometer && f*l>.5 && !sg_noclipCinematic) {
                
                glPushMatrix();

                glColor3f(1,1,1);
                
                glDisable(GL_TEXTURE_2D);

                glTranslatef(0,0,h);
                

                
                f *= 2 * sg_laggometerScale;

                glScalef(f,f,f);

                
                if (!sr_predictObjects || sn_GetNetState()==nSERVER)
                    glTranslatef(l,0,0);


                BeginLineLoop();


                glVertex2f(-l,-l);
                glVertex2f(0,0);
                glVertex2f(-l,l);
                REAL delay = GetTurnDelay();
                if(l> 2*delay){
                    glVertex2f(-2*l+delay,delay);
                    glVertex2f(-2*l+2*delay,0);
                    glVertex2f(-2*l+delay,-delay);
                }
                else if (l>delay){
                    glVertex2f(-2*l+delay,delay);
                    glVertex2f(-l,2*delay-l);
                    glVertex2f(-l,-(2*delay-l));
                    glVertex2f(-2*l+delay,-delay);
                }

                RenderEnd();
                glPopMatrix();
            }
        } else
#endif
        {
            glPushMatrix();

            
            glDisable(GL_TEXTURE_2D);

            glTranslatef(0,0,h);
            

            
            f *= 2 * sg_laggometerScale;

            glScalef(f,f,f);

            
            if (sr_predictObjects || sn_GetNetState()==nSERVER) {
                glTranslatef(-l,0,0);
            }

            
            extern bool sg_noclipCinematic;
            if (!sg_noclipCinematic)
            {
                if (f*l>sg_laggometerThreshold) {
                    if (sr_laggometer) {
                        gLaggometer::LagOMeterRenderer(this).render(l);
                    }
                } else if(sg_axesIndicator) {
                    gLaggometer::AxesIndicator(this).render();
                }
            }

            glPopMatrix();
        }
        sr_DepthOffset(false);

        glPopMatrix();

        
        
        
        
        if (sg_modPathLineEnabled && cam && cam->Center() == this) {

            
            const int   PF_ARR_SIZE  = 8;   
            int pfDepth = (sg_modPathDepth >= 2 && sg_modPathDepth <= 6) ? sg_modPathDepth : 4;
            float pfRange = (float)((sg_modPathRange >= 50 && sg_modPathRange <= 200) ? sg_modPathRange : 120);
            const float PF_MARGIN    = 2.0f;
            const float PF_FLOOR_Z   = 0.05f;

            struct PfPath {
                eCoord nodes[PF_ARR_SIZE];
                float  dists[PF_ARR_SIZE];
                int    len;
                float  totalDist;
            };

            
            PfPath bestPath;
            bestPath.len = 0;
            bestPath.totalDist = 0;

            
            struct PfFrame {
                eCoord pos;
                eCoord dir;
                float  accumulated;   
                int    depth;
                
                eCoord trail[PF_ARR_SIZE];
                float  trailD[PF_ARR_SIZE];
                int    trailLen;
            };

            
            PfFrame stack[48];
            int stackTop = 0;

            
            eCoord startPos = PredictPosition();
            eCoord startDir = Direction();
            {
                float dL = sqrt(startDir.x*startDir.x + startDir.y*startDir.y);
                if (dL > 0.001f) { startDir.x /= dL; startDir.y /= dL; }
            }

            PfFrame &seed = stack[stackTop++];
            seed.pos = startPos;
            seed.dir = startDir;
            seed.accumulated = 0;
            seed.depth = 0;
            seed.trail[0] = startPos;
            seed.trailD[0] = 0;
            seed.trailLen = 1;

            while (stackTop > 0) {
                PfFrame f = stack[--stackTop];

                float rem = pfRange - f.accumulated;
                if (rem < 1.0f || f.trailLen >= PF_ARR_SIZE) {
                    
                    if (f.accumulated > bestPath.totalDist) {
                        bestPath.totalDist = f.accumulated;
                        bestPath.len = f.trailLen;
                        for (int i = 0; i < f.trailLen; i++) {
                            bestPath.nodes[i] = f.trail[i];
                            bestPath.dists[i] = f.trailD[i];
                        }
                    }
                    continue;
                }

                
                gSensor fwd(this, f.pos, f.dir);
                fwd.detect(rem);
                float fwdHit = fwd.hit;

                
                if (fwdHit > rem * 0.9f || f.depth >= pfDepth) {
                    float adv = (fwdHit > PF_MARGIN) ? fwdHit - PF_MARGIN : fwdHit * 0.85f;
                    if (adv < 0.5f) adv = 0.5f;
                    float total = f.accumulated + adv;
                    if (total > bestPath.totalDist) {
                        bestPath.totalDist = total;
                        bestPath.len = f.trailLen + 1;
                        for (int i = 0; i < f.trailLen; i++) {
                            bestPath.nodes[i] = f.trail[i];
                            bestPath.dists[i] = f.trailD[i];
                        }
                        bestPath.nodes[f.trailLen] = f.pos + f.dir * adv;
                        bestPath.dists[f.trailLen] = total;
                    }
                    continue;
                }

                
                float advToWall = (fwdHit > PF_MARGIN) ? fwdHit - PF_MARGIN : 0;
                eCoord turnPos = f.pos + f.dir * advToWall;
                float  turnAcc = f.accumulated + advToWall;

                
                int newTrailLen = f.trailLen;
                eCoord newTrail[PF_ARR_SIZE];
                float  newTrailD[PF_ARR_SIZE];
                for (int i = 0; i < f.trailLen; i++) {
                    newTrail[i] = f.trail[i];
                    newTrailD[i] = f.trailD[i];
                }
                if (advToWall > 0.5f && newTrailLen < PF_ARR_SIZE) {
                    newTrail[newTrailLen] = turnPos;
                    newTrailD[newTrailLen] = turnAcc;
                    newTrailLen++;
                }

                
                eCoord dirs[3];
                dirs[0] = eCoord(-f.dir.y,  f.dir.x); 
                dirs[1] = eCoord( f.dir.y, -f.dir.x); 
                dirs[2] = f.dir;                        

                float branchRem = pfRange - turnAcc;
                if (branchRem < 1.0f) {
                    
                    if (turnAcc > bestPath.totalDist && newTrailLen > 0) {
                        bestPath.totalDist = turnAcc;
                        bestPath.len = newTrailLen;
                        for (int i = 0; i < newTrailLen; i++) {
                            bestPath.nodes[i] = newTrail[i];
                            bestPath.dists[i] = newTrailD[i];
                        }
                    }
                    continue;
                }

                for (int b = 0; b < 3 && stackTop < 46; b++) {
                    
                    gSensor probe(this, turnPos, dirs[b]);
                    probe.detect(branchRem);
                    if (probe.hit < PF_MARGIN * 1.5f) continue; 

                    
                    if (b == 2 && probe.hit < PF_MARGIN * 4.0f) continue;

                    PfFrame &nf = stack[stackTop++];
                    nf.pos = turnPos;
                    nf.dir = dirs[b];
                    nf.accumulated = turnAcc;
                    nf.depth = f.depth + 1;
                    nf.trailLen = newTrailLen;
                    for (int i = 0; i < newTrailLen; i++) {
                        nf.trail[i] = newTrail[i];
                        nf.trailD[i] = newTrailD[i];
                    }
                }

                
                if (turnAcc > bestPath.totalDist && newTrailLen > 0) {
                    bestPath.totalDist = turnAcc;
                    bestPath.len = newTrailLen;
                    for (int i = 0; i < newTrailLen; i++) {
                        bestPath.nodes[i] = newTrail[i];
                        bestPath.dists[i] = newTrailD[i];
                    }
                }
            }

            
            if (bestPath.len > 1) {
                glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_LIGHTING);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                float pulse = 0.75f + 0.25f * sin(tSysTimeFloat() * 3.0f);
                float td = bestPath.totalDist;

                
                glLineWidth(6.0f);
                glBegin(GL_LINE_STRIP);
                for (int i = 0; i < bestPath.len; i++) {
                    float t = (td > 0.01f) ? bestPath.dists[i] / td : 0;
                    glColor4f(0.2f, 0.7f, 1.0f, pulse * 0.12f * (1.0f - t));
                    glVertex3f(bestPath.nodes[i].x, bestPath.nodes[i].y, PF_FLOOR_Z);
                }
                glEnd();

                
                glLineWidth(2.5f);
                glBegin(GL_LINE_STRIP);
                for (int i = 0; i < bestPath.len; i++) {
                    float t = (td > 0.01f) ? bestPath.dists[i] / td : 0;
                    float a = pulse * (1.0f - t * 0.65f);
                    glColor4f(0.2f + t*0.4f, 0.85f - t*0.35f, 1.0f, a);
                    glVertex3f(bestPath.nodes[i].x, bestPath.nodes[i].y, PF_FLOOR_Z);
                }
                glEnd();

                
                for (int i = 1; i < bestPath.len - 1; i++) {
                    float dx = bestPath.nodes[i+1].x - bestPath.nodes[i].x;
                    float dy = bestPath.nodes[i+1].y - bestPath.nodes[i].y;
                    float l = sqrt(dx*dx + dy*dy);
                    if (l < 0.001f) continue;
                    float ux = dx/l, uy = dy/l;
                    float nx = -uy * 0.6f, ny = ux * 0.6f;
                    float t = bestPath.dists[i] / td;
                    float a = pulse * (1.0f - t*0.5f) * 0.7f;
                    glColor4f(1.0f, 1.0f, 0.3f, a);
                    glLineWidth(1.5f);
                    glBegin(GL_LINE_LOOP);
                    glVertex3f(bestPath.nodes[i].x + ux*0.6f, bestPath.nodes[i].y + uy*0.6f, PF_FLOOR_Z);
                    glVertex3f(bestPath.nodes[i].x + nx,      bestPath.nodes[i].y + ny,      PF_FLOOR_Z);
                    glVertex3f(bestPath.nodes[i].x - ux*0.6f, bestPath.nodes[i].y - uy*0.6f, PF_FLOOR_Z);
                    glVertex3f(bestPath.nodes[i].x - nx,      bestPath.nodes[i].y - ny,      PF_FLOOR_Z);
                    glEnd();
                }

                glPopAttrib();
            }
        }

    } else {
        
        extern bool sg_corpseTimerEnabled;
        extern bool sg_corpseTimerOverride;
        extern float sg_corpseTimerDuration;
        REAL totalDelay = gCycle::WallsStayUpDelay();
        if (totalDelay < 0.0f || sg_corpseTimerOverride) {
            totalDelay = sg_corpseTimerDuration;
        }
        if (sg_corpseTimerEnabled && totalDelay >= 0.0f) {
            REAL timeSinceDeath = se_GameTime() - deathTime;
            REAL rem = totalDelay - timeSinceDeath;
            if (rem > 0.0f && rem <= totalDelay) {
                
                float modelviewMatrix[16], projectionMatrix[16];
                float x, y, z, w;
                float xp, yp, wp;

                glPushMatrix();
                glTranslatef(0.8, 0, 2.0);
                glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);
                glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
                glPopMatrix();

                x = modelviewMatrix[12];
                y = modelviewMatrix[13];
                z = modelviewMatrix[14];
                w = modelviewMatrix[15];

                xp = projectionMatrix[0] * x + projectionMatrix[4] * y +
                     projectionMatrix[8] * z + projectionMatrix[12] * w;
                yp = projectionMatrix[1] * x + projectionMatrix[5] * y +
                     projectionMatrix[9] * z + projectionMatrix[13] * w;
                wp = projectionMatrix[3] * x + projectionMatrix[7] * y +
                     projectionMatrix[11] * z + projectionMatrix[15] * w;

                if (wp > 0) {
                    xp /= wp;
                    yp /= wp;
                    yp += rCHEIGHT_NORMAL;

                    if (xp > -1 && xp < 1 && yp > -1 && yp < 1) {
                        ModelMatrix();
                        glPushMatrix();
                        glLoadIdentity();

                        ProjMatrix();
                        glPushMatrix();
                        glLoadIdentity();

                        
                        float ratio = rem / totalDelay;
                        if (ratio > 1.0f) ratio = 1.0f;
                        if (ratio < 0.0f) ratio = 0.0f;

                        
                        float r = 1.0f - ratio;
                        float g = ratio;
                        float b = 0.1f;

                        float bar_w = 0.12f;
                        float bar_h = rCHEIGHT_NORMAL * 0.4f;
                        float bar_x = xp - bar_w * 0.5f;
                        float bar_y = yp - rCHEIGHT_NORMAL * 0.8f;

                        glDisable(GL_TEXTURE_2D);

                        
                        glColor4f(0.1f, 0.1f, 0.1f, 0.5f);
                        BeginQuads();
                        Vertex(bar_x, bar_y);
                        Vertex(bar_x + bar_w, bar_y);
                        Vertex(bar_x + bar_w, bar_y + bar_h);
                        Vertex(bar_x, bar_y + bar_h);
                        RenderEnd();

                        
                        if (ratio > 0.001f) {
                            float fill_w = bar_w * ratio;
                            glColor4f(r, g, b, 0.8f);
                            BeginQuads();
                            Vertex(bar_x, bar_y);
                            Vertex(bar_x + fill_w, bar_y);
                            Vertex(bar_x + fill_w, bar_y + bar_h);
                            Vertex(bar_x, bar_y + bar_h);
                            RenderEnd();
                        }

                        
                        glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
                        glLineWidth(1.0f);
                        BeginLineLoop();
                        Vertex(bar_x, bar_y);
                        Vertex(bar_x + bar_w, bar_y);
                        Vertex(bar_x + bar_w, bar_y + bar_h);
                        Vertex(bar_x, bar_y + bar_h);
                        RenderEnd();

                        glEnable(GL_TEXTURE_2D);

                        
                        char buf[32];
                        snprintf(buf, sizeof(buf), "%.1fs", rem);
                        tString text(buf);
                        rTextField::SetBlendColor(tColor(1.0f, 1.0f, 1.0f, 0.8f));
                        DisplayTextAutoWidth(xp, yp - rCHEIGHT_NORMAL * 1.5f, text, rCHEIGHT_NORMAL * 0.8f, 0, 0);
                        rTextField::SetBlendColor(tColor(1.0f, 1.0f, 1.0f, 1.0f));

                        ProjMatrix();
                        glPopMatrix();

                        ModelMatrix();
                        glPopMatrix();
                    }
                }
            }
        }
    }
}

static REAL fadeOutNameAfter = 5.0f;	

static bool showOwnName = 0;			

bool sg_corpseTimerEnabled = true;
static tSettingItem< bool > sg_corpseTimerConf( "CORPSE_TIMER_HUD", sg_corpseTimerEnabled );

bool sg_corpseTimerOverride = false;
static tSettingItem< bool > sg_corpseTimerOverrideConf( "CORPSE_TIMER_OVERRIDE", sg_corpseTimerOverride );

float sg_corpseTimerDuration = 8.0f;
static tSettingItem< float > sg_corpseTimerDurationConf( "CORPSE_TIMER_DURATION", sg_corpseTimerDuration );

int sg_corpseTrailStyle = 1;
static tSettingItem< int > sg_corpseTrailStyleConf( "CORPSE_TRAIL_STYLE", sg_corpseTrailStyle );

static tSettingItem< bool > sg_showOwnName( "SHOW_OWN_NAME", showOwnName );

static tSettingItem< REAL > sg_fadeOutNameAfter( "FADEOUT_NAME_DELAY", fadeOutNameAfter );


void gCycle::RenderName( const eCamera* cam ) {
    if ( !this->Player() )
        return;

    float modelviewMatrix[16], projectionMatrix[16];
    float x, y, z, w;
    float xp, yp, wp;
    float alpha = 0.75;

    if (fadeOutNameAfter == 0) return; 
    if ( !cam->RenderingMain() ) return; 
    if ( !showOwnName && cam->Player() == this->player ) return; 

    
    extern bool sg_noclipCinematic;
    extern bool sg_noclipHideNames;
    if (sg_noclipCinematic && sg_noclipHideNames) return;

    glPushMatrix();
    
    glTranslatef(0.8, 0, 2.0);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);
    glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
    glPopMatrix();

    
    x = modelviewMatrix[12];
    y = modelviewMatrix[13];
    z = modelviewMatrix[14];
    w = modelviewMatrix[15];

    
    xp = projectionMatrix[0] * x + projectionMatrix[4] * y +
         projectionMatrix[8] * z + projectionMatrix[12] * w;
    yp = projectionMatrix[1] * x + projectionMatrix[5] * y +
         projectionMatrix[9] * z + projectionMatrix[13] * w;
    wp = projectionMatrix[3] * x + projectionMatrix[7] * y +
         projectionMatrix[11] * z + projectionMatrix[15] * w;

    if (wp <= 0) {
        
        timeCameIntoView = 0;
        return;
    }

    xp /= wp;
    yp /= wp;
    yp += rCHEIGHT_NORMAL;

    if (xp <= -1 || xp >= 1 || yp <= -1 || yp >= 1) {
        
        timeCameIntoView = 0;
        return;
    }

    

    
    

    ModelMatrix();
    glPushMatrix();
    glLoadIdentity();

    ProjMatrix();
    glPushMatrix();
    glLoadIdentity();

    REAL rawRubber = this->GetRubber();
    REAL maxRubber = sg_rubberCycle;
    REAL usagePct = (maxRubber > 0.001) ? (rawRubber / maxRubber) : 0;
    if (usagePct > 1.0) usagePct = 1.0;
    if (usagePct < 0.0) usagePct = 0.0;
    
    if (sg_modRubberGaugeEnabled) {
        float ratio = usagePct;
        float r = ratio < 0.5f ? (ratio * 2.0f) : 1.0f;
        float g = ratio > 0.5f ? ((1.0f - ratio) * 2.0f) : 1.0f;
        float b = 0.0f;
        
        float bar_w = 0.12f; 
        float bar_h = rCHEIGHT_NORMAL * 0.5f; 
        float bar_x = xp - bar_w * 0.5f; 
        float bar_y = yp + rCHEIGHT_NORMAL * 1.2f; 

        glDisable(GL_TEXTURE_2D);

        
        glColor4f(0.1f, 0.1f, 0.1f, alpha * 0.6f);
        BeginQuads();
        Vertex(bar_x, bar_y);
        Vertex(bar_x + bar_w, bar_y);
        Vertex(bar_x + bar_w, bar_y + bar_h);
        Vertex(bar_x, bar_y + bar_h);
        RenderEnd();

        
        if (usagePct > 0.001f) {
            float fill_w = bar_w * usagePct;
            glColor4f(r, g, b, alpha * 0.9f);
            BeginQuads();
            Vertex(bar_x, bar_y);
            Vertex(bar_x + fill_w, bar_y);
            Vertex(bar_x + fill_w, bar_y + bar_h);
            Vertex(bar_x, bar_y + bar_h);
            RenderEnd();
        }

        
        glColor4f(1.0f, 1.0f, 1.0f, alpha);
        glLineWidth(1.0f);
        BeginLineLoop();
        Vertex(bar_x, bar_y);
        Vertex(bar_x + bar_w, bar_y);
        Vertex(bar_x + bar_w, bar_y + bar_h);
        Vertex(bar_x, bar_y + bar_h);
        RenderEnd();

        glEnable(GL_TEXTURE_2D);
    }
    
    tString nameToRender = this->player->GetColoredName();
    rTextField::SetBlendColor(tColor(1, 1, 1, alpha));
    DisplayTextAutoWidth(xp, yp, nameToRender, rCHEIGHT_NORMAL, 0, 0);
    rTextField::SetBlendColor(tColor(1, 1, 1, 1));

    ProjMatrix();
    glPopMatrix();

    ModelMatrix();
    glPopMatrix();
}


bool gCycle::RenderCockpitFixedBefore(bool){
    
    return true;
}

void gCycle::SoundMix(Uint8 *dest,unsigned int len,
                      int viewer,REAL rvol,REAL lvol){
    if (Alive()){
        

        if (engine)
            engine->Mix(dest,len,viewer,rvol,lvol,verletSpeed_/(sg_speedCycleSound * SpeedMultiplier()));

        if (turning)
        {
            if (turn_wav.alt)
                turning->Mix(dest,len,viewer,rvol,lvol,5);
            else
                turning->Mix(dest,len,viewer,rvol,lvol,1);
        }
        
        if (spark)
            spark->Mix(dest,len,viewer,rvol*.5,lvol*.5,4);
    }
}
#endif

eCoord gCycle::PredictPosition() const {
    return predictPosition_;

    
    
}

eCoord gCycle::CamPos() const
{
    return PredictPosition() + dir.Turn(0 ,-skew*z);

    
    

    

    
}

eCoord  gCycle::CamTop() const
{
    return dir.Turn(0,-skew);
}


#ifdef POWERPAK_DEB
void gCycle::PPDisplay(){
    int R=int(r*255);
    int G=int(g*255);
    int B=int(b*255);
    PD_PutPixel(DoubleBuffer,
                se_X_ToScreen(pos.x),
                se_Y_ToScreen(pos.y),
                PD_CreateColor(DoubleBuffer,R,G,B));
    
}
#endif






gCycle::gCycle(nMessage &m)
        :gCycleMovement(m),
        engine(NULL),
        turning(NULL),
        spark(NULL),
        skew(0),skewDot(0),
        rotationFrontWheel(1,0),rotationRearWheel(1,0),heightFrontWheel(0),heightRearWheel(0),
        currentWall(NULL),
        lastWall(NULL)
{
    deathTime=0;
    lastNetWall=lastWall=currentWall=NULL;
    windingNumberWrapped_ = windingNumber_ = Grid()->DirectionWinding(dirDrive);
    dirDrive = Grid()->GetDirection(windingNumberWrapped_);
    dir = dirDrive;

    rubber=0;
    
    correctDistanceSmooth =0;
    

    deathTime = 0;
    spawnTime_ = se_GameTime() + 100;

    m >> color_.r;
    m >> color_.g;
    m >> color_.b;

    trailColor_ = color_;

    se_MakeColorValid( color_.r, color_.g, color_.b, 1.0f );
    se_MakeColorValid( trailColor_.r, trailColor_.g, trailColor_.b, .5f );

    
    lastTimeAnim = lastTime = -EPS;

    nextSync = nextSyncOwner = -1;
    lastSyncOwnerGameTime_ = 0;

    
    rightTurnCount = 0;
    leftTurnCount = 0;
    for (int i = 0; i < 3; i++) {
        rightTurnTimes[i] = 0.0f;
        leftTurnTimes[i] = 0.0f;
    }
}


void gCycle::WriteCreate(nMessage &m){
    eNetGameObject::WriteCreate(m);
    m << color_.r;
    m << color_.g;
    m << color_.b;
}

static nVersionFeature sg_verletIntegration( 7 );

void gCycle::WriteSync(nMessage &m){
    

    if( SyncedUser() == Owner() )
    {
        lastSyncOwnerGameTime_ = lastTime;
    }

    if ( Alive() )
    {
        m << lastTime;
    }
    else
    {
        m << deathTime;
    }
    m << Direction();
    m << Position();

    REAL speed = verletSpeed_;
    
    if ( sg_verletIntegration.Supported() )
        speed = Speed();

#ifdef DEBUG
    if ( speed > 15 )
    {
        int x;
        x = 0;
    }
#endif

    m << speed;
    m << short( Alive() ? 1 : 0 );
    m << distance;
    if (!currentWall || currentWall->preliminary)
        m.Write(0);
    else
        m.Write(currentWall->ID());

    m.Write(turns);
    m.Write(braking);

    
    m << GetLastTurnPos();

    
    compressZeroOne.Write( m, rubber/( sg_rubberCycle + .1 ) );
    compressZeroOne.Write( m, 1/( 1 + rubberMalus ) );

    
    unsigned short lastMessageID = 0;
    if ( lastDestination )
        lastMessageID = lastDestination->messageID;
    m.Write(lastMessageID);

    
    compressZeroOne.Write( m, brakingReservoir );

    
    
    
}

bool gCycle::SyncIsNew(nMessage &m){
    bool ret=eNetGameObject::SyncIsNew(m);


    REAL dummy2;
    short al;
    unsigned short Turns;

    m >> dummy2;
    m >> al;
    m >> dummy2;
    m.Read(Turns);

#ifdef DEBUG_X
    con << "received sync for player " << player->GetName() << "  ";
    if (ret || al!=1){
        con << "accepting..\n";
        return true;
    }
    else
    {
        con << "rejecting..\n";
        return false;
    }
#endif

    return ret || al!=1;
}

void gCycle::RequestSyncOwner()
{
    
    if ( !Alive() )
    {
        return;
    }

    
    if ( sn_GetNetState() != nSERVER || Owner() == 0 )
        return;

    REAL syncInterval = sg_GetSyncIntervalSelf( this );
    if ( nextSyncOwner < tSysTimeFloat() + syncInterval * 2.0 )
    {
        
        RequestSync( Owner(), false );
        nextSyncOwner += syncInterval;
    }
}

void gCycle::RequestSyncAll()
{
    
    if ( !Alive() )
    {
        return;
    }

    
    if ( sn_GetNetState() != nSERVER || Owner() == 0 )
        return;

    REAL syncInterval = sg_syncIntervalEnemy;
    if ( nextSync < tSysTimeFloat() + syncInterval * 2.0 )
    {
        
        RequestSync( false );
        nextSync += syncInterval;
    }
}


void gCycle::ResetExtrapolator()
{
    resimulate_ = false;
    if (!extrapolator_)
    {
        extrapolator_ = tNEW( gCycleExtrapolator )(grid, pos, dir );
    }

    extrapolator_->CopyFrom( lastSyncMessage_, *this );

    
    extrapolator_->TimestepCore( extrapolator_->LastTime(), true );
}


bool gCycle::Extrapolate( REAL dt )
{
    tASSERT( extrapolator_ );

#ifdef DEBUG
    eCoord posBefore = extrapolator_->Position();
    std::ignore = posBefore;
#endif

    
    REAL newTime = extrapolator_->LastTime() + dt;

    bool ret = false;

    
    if ( newTime >= lastTime )
    {
        
        if( lastTime > extrapolator_->LastTime() )
        {
            eGameObject::TimestepThis( lastTime, extrapolator_ );
        }

        
        gDestination* unhandledDestination = extrapolator_->GetCurrentDestination();
        ret = !unhandledDestination || !unhandledDestination->list;
        
        if( !ret )
        {
            if ( unhandledDestination->gameTime < newTime - Lag() * 2 - sn_Connections[0].ping.GetPing()*2 - GetTurnDelay()*4 )
            {
                
                extrapolator_ = 0;
                resimulate_ = true;
            }
        }

        newTime = lastTime;
    }
    else
    {
        
        eGameObject::TimestepThis( newTime, extrapolator_ );
    }

    
    
    
    
    

    return ret;
}


void se_SanifyDisplacement( eGameObject* base, eCoord& displacement )
{
    eCoord base_pos = base->Position();
    

    int timeout = 5;
    while( timeout > 0 )
    {
        --timeout;

        
        gSensor test( base, base_pos, displacement );
        test.detect(1);

        if ( timeout == 0 )
        {
            
            displacement = ( displacement ) * ( test.hit * .99 );
            break;
        }

        if ( !test.ehit )
        {
            
            break;
        }
        else
        {
#ifdef DEBUG
            
            
            float p = test.ehit->Vec() * displacement;
            float m = se_EstimatedRangeOfMult( test.ehit->Vec(), displacement );
            if ( fabs(p) < m * .001 )
            {
                con << "Almost missed wall during gCycle::ReadSync positon update";
            }
#endif

            
            REAL alpha = test.ehit->Ratio( base_pos + displacement );
            displacement = *test.ehit->Point() + test.ehit->Vec() * alpha - base_pos;

            
            displacement = displacement * .99;
        }
    }
}

void gCycle::TransferPositionCorrectionToDistanceCorrection()
{
    REAL newCorrectDist = eCoord::F( correctPosSmooth, dirDrive );
    distance += newCorrectDist - correctDistanceSmooth;
    correctDistanceSmooth = newCorrectDist;
}


void gCycle::SyncFromExtrapolator()
{
    
    eCoord oldPos = pos;

    

    tASSERT( extrapolator_ );

    
    CopyFrom( *extrapolator_ );

    
    if ( currentWall && currentWall->tBeg > spawnTime_ + sg_cycleWallTime + .01f )
    {
        
        currentWall->beg = extrapolator_->GetLastTurnPos();

        
        REAL dBeg = extrapolator_->GetDistance() - eCoord::F( extrapolator_->Direction(), extrapolator_->Position() - extrapolator_->GetLastTurnPos() );

        currentWall->dbegin = dBeg;
        currentWall->coords_[0].Pos = dBeg;

        
        int i;
        for ( i = currentWall->coords_.Len() -1 ; i>=0; --i )
        {
            gPlayerWallCoord & coord = currentWall->coords_( i );
            if ( coord.Pos <= dBeg )
                coord.Pos = dBeg;
        }
    }

    
    lastTurnPos_ = extrapolator_->GetLastTurnPos();

    
    correctPosSmooth = correctPosSmooth + oldPos - pos;

#ifdef DEBUG
    if ( correctPosSmooth.NormSquared() > .1f )
    {
        std::cout << "Lag slide! " << correctPosSmooth << "\n";

    }
#endif

    
    REAL dt = this->LastTime() - extrapolator_->LastTime();

    
    REAL trueDistance = extrapolator_->trueDistance_ + extrapolator_->Speed() * dt;

    
    
    
    distance = trueDistance;
    correctDistanceSmooth=0;

    
    TransferPositionCorrectionToDistanceCorrection();

    
    

    
    
    

    
    extrapolator_ = 0;
}

static int sg_useExtrapolatorSync=1;
static tSettingItem<int> sg_useExtrapolatorSyncConf("EXTRAPOLATOR_SYNC",sg_useExtrapolatorSync);


void ClampForward( eCoord& newPos, const eCoord& startPos, const eCoord& dir )
{
    REAL forward = eCoord::F( newPos - startPos, dir )/dir.NormSquared();
    if ( forward < 0 )
        newPos = newPos - dir * forward;
}

extern REAL sg_cycleBrakeRefill;
extern REAL sg_cycleBrakeDeplete;

void gCycle::ReadSync( nMessage &m )
{
    
    SyncData sync;

    short sync_alive;               
    unsigned short sync_wall=0;     

    

    
    
    m >> sync.time;

    
    sync.rubber = rubber;
    sync.turns = turns;
    sync.braking = braking;
    sync.messageID = 1;

    m >> sync.dir;
    m >> sync.pos;

    
    
    

    m >> sync.speed;
    m >> sync_alive;
    m >> sync.distance;
    m.Read(sync_wall);
    if (!m.End())
        m.Read(sync.turns);
    if (!m.End())
        m.Read(sync.braking);

    if ( !m.End() )
    {
        m >> sync.lastTurn;
    }
    else if ( currentWall )
    {
        sync.lastTurn = currentWall->beg;
    }

    bool canUseExtrapolatorMethod = false;

    bool rubberSent = false;
    if ( !m.End() )
    {
        rubberSent = true;

        
        REAL preRubber, preRubberMalus;
        preRubber = compressZeroOne.Read( m );
        preRubberMalus = compressZeroOne.Read( m );

        
        m.Read(sync.messageID);

        
        sync.brakingReservoir = compressZeroOne.Read( m );
        

        
        sync.rubber = preRubber * ( sg_rubberCycle + .1 );
        sync.rubberMalus = 1/preRubberMalus - 1;

        
        canUseExtrapolatorMethod = sg_useExtrapolatorSync && lastTime > 0;
    }
    else
    {
        
        sync.brakingReservoir = brakingReservoir;
        if ( brakingReservoir > 0 && sync.braking )
            sync.brakingReservoir += ( lastTime - sync.time ) * sg_cycleBrakeDeplete;
        else if ( brakingReservoir < 1 && !sync.braking )
            sync.brakingReservoir -= ( lastTime - sync.time ) * sg_cycleBrakeRefill;

        if ( sync.brakingReservoir < 0 )
            sync.brakingReservoir = 0;
        else if ( sync.brakingReservoir > 1 )
            sync.brakingReservoir = 1;
    }

    
    if ( lastSyncMessage_.time >= sync.time && lastSyncMessage_.turns >= sync.turns && sync_alive > 0 )
    {
        
        
        
        return;
    }
    lastSyncMessage_ = sync;

    
    lastGoodPosition_ = sync.pos + ( sync.lastTurn - sync.pos ) *.01;
    
    if ( eCoord::F( dirDrive, sync.dir ) > .99f*dirDrive.NormSquared() )
        lastGoodPosition_ = lastGoodPosition_ - this->lastDirDrive * .0001;

    
    
    

    

    if ( canUseExtrapolatorMethod )
    {
        
        if ( extrapolator_ && extrapolator_->LastTime() < lastSyncMessage_.time )
        {
            extrapolator_ = 0;
        }
    }

    
    if (Alive() && sync_alive!=1 && GOID() >= 0 && grid )
    {
        Die( lastSyncMessage_.time );
        MoveSafely( lastSyncMessage_.pos, lastTime, deathTime );
        distance=lastSyncMessage_.distance;
        correctDistanceSmooth=0;
        DropWall( false );

        tNEW(gExplosion)( grid, lastSyncMessage_.pos, lastSyncMessage_.time ,color_, this );

        return;
    }

    
    if (!Alive())
    {
#ifdef DEBUG
        con << "Received duplicate death sync message; those things confuse old clients!\n";
#endif
        return;
    }

    gDestination emergency_aft(*this);

    
    gDestination *bef= GetDestinationBefore( lastSyncMessage_, destinationList );
    gDestination *aft=NULL;
    if (bef){
        aft=bef->next;
        if (!aft)
            aft=&emergency_aft;

        
        
        
        
        

    }


    
    
    if ( lastTime > 0 )
    {
        eCoord position = pos;
        if ( bef )
            position = bef->position;
        eSensor tunnel( this, lastSyncMessage_.pos, position - lastSyncMessage_.pos );
        tunnel.detect( 1 );

        
        if ( tunnel.ehit )
        {
            canUseExtrapolatorMethod = true;
            if ( 0 )
            {
                con << "possible local tunneling detected\n";
                eSensor tunnel( this, lastSyncMessage_.pos, position - lastSyncMessage_.pos );
                tunnel.detect( 1 );
            }
        }
    }
    else
    {
        
        pos = sync.pos;
        FindCurrentFace();
    }

    
    bool distanceBased = aft && aft != &emergency_aft && Owner() == sn_myNetID;

    if ( canUseExtrapolatorMethod && Owner()==sn_myNetID )
    {
        
        resimulate_ = true;

        return;
    }

    rubber = lastSyncMessage_.rubber;

    

    
    REAL interpolatedDistance = lastSyncMessage_.distance;
    if ( aft )
    {
        interpolatedDistance = aft->distance - sqrt((lastSyncMessage_.pos-aft->position).NormSquared());
    }

    
    
    
    

    if ( distanceBased )
    {
        

#ifdef DEBUG
        
#endif

        REAL ratio = (interpolatedDistance - bef->distance)/
                     (aft->distance - bef->distance);

        if (!std::isfinite(ratio))
            ratio = 0;

        
        REAL interpolatedTime = bef->gameTime * (1-ratio) + aft->gameTime * ratio;
        

        
        REAL correctTime  = ( lastSyncMessage_.time     - interpolatedTime     );
        
        REAL correctDist  = ( lastSyncMessage_.distance - interpolatedDistance );

        
        {
            REAL factor = (1 - ratio) * 5;
            if ( factor < 1 )
            {
                if ( factor < 0 )
                    factor = 0;
                correctTime *= factor;
                correctDist *= factor;
            }
        }

        
        
        
        
        
        

        
        
        

        

        

        
        {
            distance += correctDist;
            gDestination * run = bef;
            while ( run )
            {
                run->distance += correctDist;
                run = run->next;
            }
        }

        
        eCoord newPos = pos - Direction() * Speed() * correctTime;
        if ( currentWall )
        {
            ClampForward( newPos, currentWall->beg, Direction() );
        }

        
        {
            const eCoord & safePos = pos; 
            gSensor test( this, safePos , newPos - safePos );
            test.detect(1);
            if ( test.ehit )
            {
                newPos = test.before_hit;

                
                resimulate_ = true;
            }
        }

        correctPosSmooth = correctPosSmooth + pos - newPos;
        distance += eCoord::F( newPos - pos, Direction() )/Direction().NormSquared();

        MoveSafely( newPos, lastTime, lastTime );

        
    }
    else
    {
        
        if ( Owner() != sn_myNetID )
        {
            
            SyncEnemy( lastSyncMessage_.lastTurn );

            
            if ( currentWall )
            {
                currentWall->beg = lastSyncMessage_.lastTurn;
            }

            
            AccelerationDiscontinuity();
            braking = lastSyncMessage_.braking;

            
            lastTurnPos_ = lastSyncMessage_.lastTurn;
        }
        else
        {
            
            eCoord oldPos = pos + correctPosSmooth;
            SyncEnemy( lastSyncMessage_.lastTurn );
            correctPosSmooth = oldPos - pos;
        }

        
        if ( !rubberSent )
        {
            rubber = lastSyncMessage_.rubber;
        }
    }

    
    if ( this->ID() == 0 )
    {
        correctPosSmooth = eCoord();

        

        
        spawnTime_=lastTime;
        if ( verletSpeed_ > 0 )
            spawnTime_ -= distance/verletSpeed_;

        
        if ( !sg_cycleFirstSpawnProtection && spawnTime_ <= 1.0 )
        {
            spawnTime_ = -1E+20;
        }

        
        predictPosition_ = pos;
        dir = dirDrive;
        skew = skewDot = 0;
        lastDirDrive=dirDrive;
        lastTurnPos_=pos;
    }
#ifdef DEBUG
    else
        if ( correctPosSmooth.NormSquared() > .1f && lastTime > 0.0 )
        {
            std::cout << "Lag slide! " << correctPosSmooth << "\n";
            int x;
            x = 0;
        }
#endif

    sn_Update(turns,lastSyncMessage_.turns);

    
    

    
    this->SetWindingNumberWrapped( Grid()->DirectionWinding(dirDrive) );

    
    dirDrive = Grid()->GetDirection(windingNumberWrapped_);
}

void gCycle::SyncEnemy ( const eCoord& )
{
    
    tJUST_CONTROLLED_PTR< gCycle > keep( this->GetRefcount()>0 ? this : 0 );

    resimulate_ = false;

    
    bool turned = false;
    REAL turnDirection=( dirDrive*lastSyncMessage_.dir );
    REAL notTurned=eCoord::F( dirDrive, lastSyncMessage_.dir )/dirDrive.NormSquared();

    
    REAL lastKnownTime = lastTime;

    
    if ( distance > 0 && ( notTurned < .99 || this->turns < lastSyncMessage_.turns ) )
    {
        
        if (turning)
            turning->Reset();

        
        eCoord crossPos = lastSyncMessage_.pos;
        REAL crossDist = lastSyncMessage_.distance;
        REAL crossTime = lastSyncMessage_.time;

        
        
        
        
        if (this->turns+1 >= lastSyncMessage_.turns && ( lastSyncMessage_.turns > 0 || notTurned > -.5 ) )
        {
            if ( fabs( turnDirection ) > .01 )
            {
                REAL b = ( crossPos - pos ) * dirDrive;
                REAL distplace = b/turnDirection;
                crossPos = crossPos + lastSyncMessage_.dir * distplace;
                crossDist += distplace;
                if ( lastSyncMessage_.speed > 0 )
                    crossTime += distplace / lastSyncMessage_.speed;

                tASSERT( fabs ( ( crossPos - pos ) * dirDrive ) < 1 );
                tASSERT( fabs ( ( crossPos - lastSyncMessage_.pos ) * lastSyncMessage_.dir ) < 1 );

                
                if (currentWall)
                    currentWall->Update(crossTime,crossPos);
            }
        }
        else
        {
            
            
            if (currentWall)
            {
                currentWall->real_CopyIntoGrid(grid);
            }
        }

        eDebugLine::SetTimeout(5);
        eDebugLine::SetColor( 1,0,0 );
        eDebugLine::Draw( crossPos, 0, crossPos, 8 );

        
        if(currentWall){
            lastWall=currentWall;
            currentWall->CopyIntoGrid( grid );
            tControlledPTR< gNetPlayerWall > bounce( currentWall );
            currentWall=NULL;
        }

        
        distance = lastSyncMessage_.distance;
        correctDistanceSmooth=0;

        REAL startBuildWallAt = spawnTime_ + sg_cycleWallTime;
        if ( crossTime > startBuildWallAt )
            currentWall=new gNetPlayerWall
                        (this,crossPos,lastSyncMessage_.dir,crossTime,crossDist);

        turned = true;

        
        lastDirDrive = dirDrive;

        
        MoveSafely( crossPos, lastTime, crossTime );
        lastKnownTime = crossTime;
    }

    
    skewDot+=4*turnDirection;

    
    

    
    REAL oldTime = lastTime;

    
    MoveSafely( lastSyncMessage_.pos, lastKnownTime, lastSyncMessage_.time );
    verletSpeed_  = lastSyncMessage_.speed;
    lastTimestep_ = 0;
    distance = lastSyncMessage_.distance;
    correctDistanceSmooth=0;
    dirDrive = lastSyncMessage_.dir;
    rubber = lastSyncMessage_.rubber;
    brakingReservoir = lastSyncMessage_.brakingReservoir;

    
    lastTime = lastSyncMessage_.time;
    if ( oldTime < 0 )
        oldTime = lastTime;
    clientside_action();

    
    if ( lastDirDrive * dirDrive < EPS && eCoord::F( lastDirDrive, dirDrive ) < 0 )
    {
        lastDirDrive = dirDrive.Turn(0,1);
    }

    
    if (Owner() != sn_myNetID )
    {
        
        REAL lag = se_GameTime() - lastSyncMessage_.time;
        if ( lag < 0 )
            lag = 0;

        
        REAL maxLag = laggometer * 1.2;
        REAL minLag = laggometer * .8;

        
        laggometer = lag;

        
        
        

        
        
        
        if (
            sn_GetNetState()==nCLIENT && turned )
        {
            laggometerSmooth = lag;

            
            if ( currentWall )
                currentWall->Update(lastTime,pos);

            
            lastTimeAnim=lastTime;

            return;
        }
        else
        {
            
            
            
            if ( laggometer > maxLag )
                laggometer = maxLag;
            if ( laggometer < minLag )
                laggometer = minLag;
        }
    }

    
    {
        REAL laggometerSmoothBackup = this->laggometerSmooth;
        TimestepThis(oldTime, this);
        this->laggometerSmooth = laggometerSmoothBackup;
    }
}



void gCycle::ReceiveControl(REAL time,uActionPlayer *act,REAL x){
    
    TimestepThis(time,this);
    Act(act,x);
    
}

void gCycle::PrintName(tString &s) const
{
    s << "gCycle nr. " << ID();
    if ( this->player )
    {
        s << " owned by ";
        this->player->PrintName( s );
    }
}

bool gCycle::ActionOnQuit()
{
    
    
    if ( sn_GetNetState() == nSERVER )
    {
        TakeOwnership();
        Kill();
        return false;
    }
    else
    {
        return true;
    }
}


nDescriptor &gCycle::CreatorDescriptor() const{
    return cycle_init;
}



void gCycle::RightBeforeDeath( int numTries )
{
    if ( player )
    {
        player->RightBeforeDeath( numTries );
    }

    
    if ( sg_avoidBadOldClientSync && !sg_NoLocalTunnelOnSync.Supported( Owner() ) )
    {
        nextSync=tSysTimeFloat()+sg_syncIntervalEnemy;
        nextSyncOwner=tSysTimeFloat()+sg_GetSyncIntervalSelf( this );
    }

    correctPosSmooth = correctPosSmooth * .5;
}












bool gCycle::DoIsDestinationUsed( const gDestination * dest ) const
{
    return ( extrapolator_ && extrapolator_->IsDestinationUsed( dest ) ) || gCycleMovement::DoIsDestinationUsed( dest );
}



































bool gCycle::Vulnerable() const
{
#ifndef DEDICATED
    if (DemoPlayerManager::Instance().IsViewerActive() && DemoPlayerManager::Instance().IsLoaded()) return false;
#endif
    return Alive() && lastTime > spawnTime_ + sg_cycleInvulnerableTime;
}
