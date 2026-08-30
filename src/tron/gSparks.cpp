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

#include "gSparks.h"
#include "eWorldPaint.h"
#include "eTimer.h"
#include "rRender.h"
#include "tRandom.h"

bool white_sparks=false;

gSpark::gSpark(eGrid *grid, const eCoord &pos,const eCoord &dir,REAL time,REAL ocolor_r,REAL ocolor_g,REAL ocolor_b,REAL ecolor_r,REAL ecolor_g,REAL ecolor_b)
        :eReferencableGameObject(grid, pos, dir , NULL, true),
        //   sound(scrap),
createTime(time){
    lastTime=createTime;

    sparkowncolor_r=ocolor_r;
    sparkowncolor_g=ocolor_g;
    sparkowncolor_b=ocolor_b;

    sparkenemycolor_r=ecolor_r;
    sparkenemycolor_g=ecolor_g;
    sparkenemycolor_b=ecolor_b;

    for (int i=SPARKS-1;i>=0;i--){
        lastX[i]=preLastX[i]=x[i]=Vec3(pos.x,pos.y,.5);

        static const REAL fak=4;

        tRandomizer & randomizer = tRandomizer::GetInstance();
        REAL a=fak*( randomizer.Get() - .5f );
        REAL b=fak*( randomizer.Get() - .5f );
        //      REAL a=fak*(rand()/static_cast<REAL>(RAND_MAX)-.5f);
        //      REAL b=fak*(rand()/static_cast<REAL>(RAND_MAX)-.5f);
        REAL c=1;

        eCoord xy(eCoord(c,b).Turn(dir));

        xDot[i]=Vec3(xy.x,xy.y,a);
        xDot[i]=xDot[i]*(1/xDot[i].Norm());
        xDot[i].x[2]+=1;

        heat[i]=2+randomizer.Get();
        //      heat[i]=2+rand()/REAL(RAND_MAX);
        lastBreak[i]=createTime;
    }

    // add to game grid
    this->AddToList();
}

gSpark::~gSpark(){}

// virtual eGameObject_type type();

bool gSpark::Timestep(REAL currentTime){
    REAL ts=currentTime-lastTime;
    lastTime=currentTime;

    for (int i=SPARKS-1;i>=0;i--){
        x[i]+=xDot[i]*ts;
        xDot[i].x[2]-=5*ts;
        heat[i]-=ts;

        if (x[i].x[2]<0){
            x[i].x[2]*=-1;
            xDot[i].x[2]*=-.5;
            lastBreak[i]=currentTime;
        }
    }

    if (currentTime>createTime+4)
        return true;
    else
        return false;

}

void gSpark::InteractWith(eGameObject *,REAL ,int){}
void gSpark::PassEdge(const eWall *,REAL ,REAL ,int){}

void gSpark::Kill(){createTime=lastTime-100000;}


#ifndef DEDICATED
void gSpark::Render(const eCamera *cam){
    // sparks answer to the game's own sparks setting, which is what put them
    // here in the first place; the master switch had been overruling it
    extern bool crash_sparks;
    if (!crash_sparks) return;
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);

    //glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();
    //glLoadIdentity();

    //glDisable(GL_TEXTURE);
    glDisable(GL_TEXTURE_2D);

    bool hearts = rc_HeartSparks();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive glowing electric sparks

    if (hearts)
    {
        // Only as many as were asked for, spread across the whole burst rather
        // than taken from one end of it, so thinning them out keeps the shape.
        int howMany = rc_HeartSparkCount();
        int step = SPARKS / howMany;
        if ( step < 1 ) step = 1;

        BeginTriangles();
        for (int i=SPARKS-1;i>=0;i-=step){
            REAL a = heat[i] * 0.7f;
            if (a > 0.9f) a = 0.9f;
            if (a <= 0.0f) continue;

            // Small, and the same size throughout. These come off a wall by
            // the dozen, so anything larger stops being a spark and becomes
            // something in the way.
            rc_HeartSolidInto( x[i].x[0], x[i].x[1], x[i].x[2],
                           0.10f + 0.06f * heat[i],
                           (REAL)i * 0.9f,
                           a );
        }
        RenderEnd();
    }
    else
    {
        glLineWidth(1.8f);
        BeginLines();
        for (int i=SPARKS-1;i>=0;i--){
            REAL a = heat[i] * 0.5f;
            if (a > 0.8f) a = 0.8f;
            if (a < 0.0f) a = 0.0f;

            // Elegant electric cyan / magenta arc lightning colors
            if (i % 2 == 0) {
                glColor4f(0.0f, 0.85f, 1.0f, a); // Electric Cyan
            } else {
                glColor4f(0.9f, 0.2f, 1.0f, a);  // Electric Purple/Magenta
            }

            x[i].RenderVertex();
            preLastX[i] = x[i];
            preLastX[i] += xDot[i] * (-0.10f);
            preLastX[i].RenderVertex();
        }
        RenderEnd();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glPopMatrix();

}

void gSpark::SoundMix(Uint8 *dest,unsigned int len,
                      int viewer,REAL rvol,REAL lvol){
    //  sound.Mix(dest,len,viewer,rvol*.5,lvol*.5,4);
}
#endif
