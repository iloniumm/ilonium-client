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

#include "gExplosion.h"
#include "eWorldPaint.h"
#include "rModel.h"
#include "rRender.h"
#include "tInitExit.h"
#include "gWall.h"
#include "gCycle.h"
#include "eGrid.h"
#include "tRandom.h"
#include "tMath.h"

static eWavData explode("moviesounds/dietron.wav","sound/expl.wav");

static tList< gExplosion > sg_Explosions;

static void clamp01(REAL &c)
{
    if (!std::isfinite(c))
        c = 0.5;

    if (c<0)
        c = 0;

    if (c>1)
        c = 1;
}

int	gExplosion::expansionSteps=5;
REAL gExplosion::expansionTime = 0.2f;

static eCoord s_explosionCoord;
static REAL   s_explosionRadius;
static REAL	  s_explosionTime;
static gExplosion * s_holer = 0;

// blow a hole centered at s_explosionCoord with radius s_explosionRadius into wall w
static void S_BlowHoles( eWall * w )
{
    // determine the point closest to s_explosionCoord
    eCoord normal = w->Vec().Conj();
    normal.Normalize();

    eCoord Pos1 = normal.Turn( w->EndPoint(0) - s_explosionCoord );
    eCoord Pos2 = normal.Turn( w->EndPoint(1) - s_explosionCoord );

    tASSERT( fabs( Pos1.y - Pos2.y ) <= fabs( Pos1.y + Pos2.y + .1f ) * .1f );

    REAL alpha = .5f;
    if ( Pos1.x != Pos2.x)
        alpha = Pos1.x / ( Pos1.x - Pos2.x );

    REAL radius = s_explosionRadius * s_explosionRadius - Pos1.y * Pos2.y;

    // wall too far away
    if ( radius < 0 )
        return;

    radius = sqrt( radius );

    // works only for player walls
    gPlayerWall* wall = dynamic_cast<gPlayerWall*>( w );
    if ( ! wall )
        return;

    REAL closestPos = wall->Pos( alpha );

    REAL start = closestPos - radius;
    REAL end = closestPos + radius;

    // only cut away walls that were created before the explosion started
    REAL wallEnd = wall->EndPos();
    REAL wallBeg = wall->BegPos();
    if ( wallEnd <= wallBeg )
    {
        return;
    }
    REAL endAlpha = ( end - wallBeg ) / ( wallEnd - wallBeg );
    // std::cout << wall->BegTime() << "\n";
    clamp01( endAlpha );
    REAL endHoleTime = wall->Time( endAlpha );
    if ( endHoleTime > s_explosionTime )
    {
        REAL begTime = wall->BegTime();
        REAL endTime = wall->EndTime();
        REAL timeNorm = endTime - begTime;
        if ( timeNorm != 0.0f )
        {
            endAlpha = ( s_explosionTime - begTime ) / timeNorm;
            end = wall->Pos( endAlpha );
        }
    }

    if ( end > start )
    {
        wall->BlowHole ( start, end, s_holer );
    }
}

/*
// synchronize walls affected by explosion
static void S_Sync( eWall * w )
{
	// works only for player walls
	gPlayerWall* wall = dynamic_cast<gPlayerWall*>( w );
	if ( ! wall )
		return;

	wall->RequestSync ();
}
*/

gExplosion::gExplosion(eGrid *grid, const eCoord &pos,REAL time, gRealColor& color, gCycle * owner )
        :eReferencableGameObject(grid, pos, eCoord(0,0), NULL, true),
        sound(explode),
        createTime(time),
        expansion(0),
        listID(-1),
        owner_(owner) 
{
    holeAccountedFor_ = false;

    lastTime = time;
    explosion_r = color.r;
    explosion_g = color.g;
    explosion_b = color.b;
    sound.Reset();
    sg_Explosions.Add( this, listID );
    z=0;

    theExplosion = 0;
#ifdef USE_PARTICLES
    ParticleSystem expParams;

    expParams.particle_size = 10.0f;  // Obviously, the size of the particles
    expParams.numParticles = 1000;     // Total number of particles in the system
    expParams.initialSpread = 1.0f;    // Initial spread of particles
    expParams.gravity = 2.0f;        // (Gravity)
    expParams.generateNewParticles = false; // Flag, if false don't generate new particles
    expParams.numStartParticles = 1000;  // Number of active particles to initialize with
    expParams.life = 10000.0f;           // Average lifetime of a particle
    expParams.lifeRand = 1.03f;       // Factor to use to randomize particle lifetime
    expParams.expansion = 1.0f;      // The rate at which the system will run
    expParams.flow = 1.0f;

    glCoord theVector;

    theVector.x = 0;
    theVector.y = 0;
    theVector.z = 2;

    theExplosion = new gParticles(pos, theVector, time, expParams);
#endif

    // add to game object lists
    AddToList();
}

gExplosion::~gExplosion(){
    sg_Explosions.Remove( this, listID );

    if ( s_explosionRadius > 0 )
    {
        /*
        s_explosionCoord  = pos;
        s_explosionRadius = gCycle::ExplosionRadius()*1.5f;

        grid->ProcessWallsInRange( &S_Sync,
        						   s_explosionCoord,
        						   s_explosionRadius,
        						   this->CurrentFace() );
        */
    }
    delete theExplosion;
}

// virtual eGameObject_type type();


bool gExplosion::Timestep(REAL currentTime){
    lastTime=currentTime;

    int currentExpansion = int(REAL( expansionSteps )*((currentTime - createTime)/expansionTime)) + 1;
    if ( currentExpansion > expansionSteps )
        currentExpansion = expansionSteps;

    if ( currentExpansion > expansion )
    {
        expansion = currentExpansion;

        s_explosionCoord  = pos;
        REAL factor = expansion / REAL( expansionSteps );
        s_explosionRadius = gCycle::ExplosionRadius() * sqrt(factor);
        s_explosionTime = currentTime;
        s_holer = this;

        if ( s_explosionRadius > 0 && (currentTime < createTime+4) )
        {
            grid->ProcessWallsInRange( &S_BlowHoles,
                                       s_explosionCoord,
                                       s_explosionRadius,
                                       this->CurrentFace() );
        }

        s_holer = 0;
    }

    /*
    	// see every once in a while if all cycles did a turn since the expl was created
    	if ( expansion >= expansionSteps )
    	{
    		currentExpansion = expansionSteps + 1 + int( REAL( expansion ) * ( currentTime - createTime ) );
    	}
    	if ( currentExpansion > expansion )
    	{
    		expansion = currentExpansion;
    		REAL time = createTime + 5.0f;
    		
    		const tList<eGameObject>& gameObjects = this->Grid()->GameObjects();
    		for ( int i = gameObjects.Len() - 1; i>=0; --i )
    		{
    			eGameObject* o = gameObjects( i );

    			gCycle* c = dynamic_cast< gCycle* >( o );
    			if ( c && c->Alive() )
    			{
    				const gPlayerWall* w = c->CurrentWall();

    				if ( !w	|| w->BegTime() < time || w->EndTime() < time )
    				{
    					// c has not made a turn; we need to stay around some more
    					return false;
    				}
    			}
    		}
    	}
    */
#ifdef USE_PARTICLES
    return theExplosion->Timestep(currentTime);
#else
    if (currentTime>createTime+4)
        return true;
    else
        return false;
#endif
}

void gExplosion::InteractWith(eGameObject *,REAL ,int){}

void gExplosion::PassEdge(const eWall *,REAL ,REAL ,int){}

void gExplosion::Kill(){
    createTime=lastTime-100;
}

bool gExplosion::AccountForHole()
{
    bool ret = !holeAccountedFor_;
    holeAccountedFor_ = true;
    return ret;
}

void gExplosion::OnNewWall( eWall* w )
{
    for ( int i = sg_Explosions.Len() - 1; i>=0; --i )
    {
        const gExplosion* e = sg_Explosions(i);
        if ( e->expansion >= expansionSteps )
        {
            // e is a already completed explosion
            s_explosionCoord  = e->pos;
            s_explosionRadius = gCycle::ExplosionRadius();
            s_explosionTime = e->createTime;

            S_BlowHoles( w );
        }
    }
}

static tArray<Vec3> expvec;

static void init_exp(){
    int i=0;
    expvec[i++]=Vec3(0,0,1);
    expvec[i++]=Vec3(0,1,1);
    expvec[i++]=Vec3(0,-1,1);
    expvec[i++]=Vec3(1,0,1);
    expvec[i++]=Vec3(-1,0,1);
    expvec[i++]=Vec3(1,1,1);
    expvec[i++]=Vec3(-1,1,1);
    expvec[i++]=Vec3(1,-1,1);
    expvec[i++]=Vec3(-1,-1,1);

    const REAL fak=7;

    tRandomizer & randomizer = tRandomizer::GetInstance();

    for (int j=i;j<40;j++){
        expvec[i++]=Vec3(fak*( randomizer.Get() -.5f ),
                         fak*( randomizer.Get() -.5f ),
                         1);
        //        expvec[i++]=Vec3(fak*(rand()/static_cast<REAL>(RAND_MAX)-.5f),
        //                         fak*(rand()/static_cast<REAL>(RAND_MAX)-.5f),
        //                         1);
    }

    for (int k=expvec.Len()-1;k>=0;k--)
        expvec[k]=expvec[k]*(1/expvec[k].Norm());
}

static tInitExit ie_exp(&init_exp);

bool sg_crashExplosion = true;
bool sg_explosionSingleLineUp = false;

#ifndef DEDICATED
void gExplosion::Render(const eCamera *cam){
#ifdef USE_PARTICLES
    if (sg_crashExplosion){
        theExplosion->Render(cam);
    }
#else
    if (sg_crashExplosion){
        REAL a1=(lastTime-createTime)+.01f;//+.2;
        REAL e=a1-1;

        if (e<0) e=0;

        REAL fade=(2-a1);
        if (fade<0) fade=0;
        if (fade>1) fade=1;

        a1*=100;
        e*=100;

        ModelMatrix();
        glPushMatrix();
        glTranslatef(pos.x,pos.y,0);

        //glDisable(GL_TEXTURE);
        glDisable(GL_TEXTURE_2D);

        glColor4f(explosion_r,explosion_g,explosion_b,fade);

        // A crash can throw hearts instead of shards.
        //
        // They are thrown, they fall, they bounce and they fade where they
        // land. The blast's own radii are no use for that - one of them covers
        // a hundred units a second - so the flight is worked out from the age
        // of the explosion instead, which is the only thing here that means
        // anything physical.
        //
        // Worked out rather than stored: this is called once per viewport and
        // the answer has to be the same in each, and a formula cannot drift
        // out of step with itself the way a running simulation can.
        if ( rc_HeartDeaths() )
        {
            REAL age = lastTime - createTime;

            REAL const gravity = 26.0f;
            REAL const bounce = 0.42f;      // how much of the fall comes back
            REAL const drag = 0.55f;        // how quickly they stop travelling
            // The explosion itself is gone at two seconds. Anything longer
            // than that would not fade out, it would be cut off.
            REAL const lifetime = 1.85f;

            int howMany = rc_HeartDeathCount();

            if ( age < lifetime )
            {
                BeginTriangles();

                for ( int i = howMany - 1; i >= 0; --i )
                {
                    // More hearts than the blast has directions is normal, so
                    // the extras are turned off the ones it does have rather
                    // than piling up on top of them.
                    int from = i % expvec.Len();
                    REAL swing = (REAL)( i / expvec.Len() ) * 0.7f;
                    REAL sway = cosf( swing ), tilt = sinf( swing );

                    REAL dx = expvec[from].x[0] * sway - expvec[from].x[1] * tilt;
                    REAL dy = expvec[from].x[0] * tilt + expvec[from].x[1] * sway;

                    // Thrown properly. The first version barely cleared the
                    // ground, which is why it looked like something failing
                    // rather than something happening.
                    REAL speed = 3.4f + 1.3f * ( i % 5 );
                    REAL up = 7.5f + 2.4f * ( i % 4 );

                    // Sideways, slowing to a stop rather than carrying on.
                    REAL travelled = ( 1.0f - expf( -age / drag ) ) * drag;
                    REAL px = dx * speed * travelled;
                    REAL py = dy * speed * travelled;

                    // Up, down, and up again a little less each time.
                    REAL v = up;
                    REAL left = age;
                    REAL height = 0.0f;

                    for ( int arc = 0; arc < 6; ++arc )
                    {
                        REAL flight = 2.0f * v / gravity;
                        if ( left < flight )
                        {
                            height = v * left - 0.5f * gravity * left * left;
                            break;
                        }
                        left -= flight;
                        v *= bounce;
                    }

                    if ( height < 0.0f )
                        height = 0.0f;

                    // Turning while it is in the air and settling once it is
                    // down, so a heart at rest stops spinning on the spot.
                    REAL turn = (REAL)i * 0.9f + age * ( height > 0.05f ? 2.2f : 0.0f );

                    // Fades over the last third of its life rather than
                    // blinking out.
                    REAL gone = age / lifetime;
                    REAL alpha = gone > 0.65f ? ( 1.0f - gone ) / 0.35f : 1.0f;

                    rc_HeartSolidInto( px, py, height + 0.18f, 0.16f, turn, alpha );
                }

                RenderEnd();
            }
        }
        else
        {
            BeginLines();
            if (sg_explosionSingleLineUp) {
                glVertex3f(0.0f, 0.0f, a1);
                glVertex3f(0.0f, 0.0f, e);
            } else {
                for (int i=expvec.Len()-1;i>=0;i--){
                    glVertex3f(a1*expvec[i].x[0],a1*expvec[i].x[1],a1*expvec[i].x[2]);
                    glVertex3f( e*expvec[i].x[0], e*expvec[i].x[1], e*expvec[i].x[2]);
                }
            }
            RenderEnd();
        }
        glPopMatrix();
    }
    /*
    if(sr_alphaBlend)
    for(int i=2;i>=0;i--){
    REAL age=a1-i*.1;
    if (0<age && age<.5){
    REAL alpha=(3-i)*.5*(1-age*2)*(1-age*2);
    glColor4f(r,g,b,alpha);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(pos.x,pos.y,age*pow((age+1),3)*(age+.5)*100);
    y

    GLUquadricObj* q= gluNewQuadric();

    gluSphere(q,(age*pow((age+1),3)*(age+.5))*100,5,5);

    gluDeleteQuadric( q );

    glPopMatrix();
    }
    }
    */
#endif
}

void gExplosion::SoundMix(Uint8 *dest,unsigned int len,
                          int viewer,REAL rvol,REAL lvol){
    sound.Mix(dest,len,viewer,rvol*4,lvol*4);
}
#endif

void gExplosion::OnRemoveFromGame()
{
    // remove from list to avoid costy checks whenever a new wall appears
    sg_Explosions.Remove( this, listID );

    // delegate to base
    eReferencableGameObject::OnRemoveFromGame();
}

