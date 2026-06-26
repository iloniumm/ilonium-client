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


#include "defs.h"
#ifndef DEDICATED
#include "rSDL.h"
#endif

#include "rSysdep.h"
#include "tInitExit.h"
#include "tDirectories.h"
#include "tSysTime.h"
#include "rConsole.h"
#include "config.h"
#include <iostream>
#include "rScreen.h"
#include "rGL.h"
#include "tCommandLine.h"
#include "tConfiguration.h"
#include "tRecorder.h"

#ifndef DEDICATED
// SDL3: SDL_thread.h and SDL_mutex.h are now included via <SDL3/SDL.h>
// (already included transitively through rSDL.h)

// // #include <png.h>
#define SCREENSHOT_PNG_BITDEPTH 8
#define SCREENSHOT_BYTES_PER_PIXEL 3
#ifndef SDL_OPENGL
#ifndef DIRTY
#define DIRTY
#endif
#endif

//#ifndef SDL_OPENGL
//#error "need SDL 1.1"
//#endif

#ifndef DIRTY

// nothing to be done.

/*
//#elif defined(HAVE_FXMESA)
 #include <GL/gl>
 #include <GL/fxmesa>

 static fxMesaContext ctx=NULL;
*/

#elif defined(WIN32)

 #include <windows.h>
 #include <windef.h>
 #include "rGL.h"
static HDC hDC=NULL;
static HGLRC hRC=NULL;

#elif defined(unix) || defined(__unix__)

#include <GL/glx.h>
static GLXContext cx;
Display *dpy=NULL;
Window  win;

#endif

#ifdef DIRTY
#include <SDL3/SDL.h>

bool  rSysDep::InitGL(){
    return true;
}

void  rSysDep::ExitGL(){
}
#endif // DIRTY

bool sr_screenshotIsPlanned=false;

static bool png_screenshot=true;
static tConfItem<bool> pns("PNG_SCREENSHOT",png_screenshot);
#ifndef DEDICATED

#endif

static void SDL_SavePNG(SDL_Surface *image, tString filename){
    fprintf(stderr, "PNG screenshots disabled, use BMP.\n");
}

static void make_screenshot(){
#ifndef DEDICATED
    // screenshot count
    static int number=0;
    number++;

    SDL_Surface *image;
    SDL_Surface *temp;
    int idx;
    // SDL3: use SDL_CreateSurface with a pixel format enum
    image = SDL_CreateSurface(sr_screenWidth, sr_screenHeight, SDL_PIXELFORMAT_RGB24);
    temp  = SDL_CreateSurface(sr_screenWidth, sr_screenHeight, SDL_PIXELFORMAT_RGB24);

    // make upside down screenshot
    glReadPixels(0,0,sr_screenWidth, sr_screenHeight, GL_RGB,
                 GL_UNSIGNED_BYTE, image->pixels);

    // turn image around
    for (idx = 0; idx < sr_screenHeight; idx++)
    {
        memcpy(reinterpret_cast<char *>(temp->pixels) + temp->pitch * idx,
               reinterpret_cast<char *>(image->pixels)
               + image->pitch*(sr_screenHeight - idx-1),
               3*sr_screenWidth); // Optionally, use the pitch of either surface here
    }

    // save screenshot in unused slot
    bool done = false;
    while ( !done )
    {
        // generate filename
        tString fileName("screenshot_");
        fileName << number;
        if(png_screenshot)
            fileName << ".png";
        else
            fileName << ".bmp";

        // test if file exists
        std::ifstream s;
        if ( tDirectories::Screenshot().Open( s, fileName ) )
        {
            // yes! try next number
            number++;
            continue;
        }

        // save image
        if(png_screenshot)
            SDL_SavePNG(image, tDirectories::Screenshot().GetWritePath( fileName ));
        else
            SDL_SaveBMP(temp, tDirectories::Screenshot().GetWritePath( fileName ) );
        done = true;
    }

    // cleanup
    SDL_DestroySurface(image);
    SDL_DestroySurface(temp);
#endif
}

class PerformanceCounter
{
public:
    PerformanceCounter(): count_(0){ tRealSysTimeFloat(); }
    unsigned int Count(){ return count_++; }
    ~PerformanceCounter()
    {
        double time = tRealSysTimeFloat();
        std::stringstream s;
        s << count_ << " frames in " << time << " seconds: " << count_ / time << " fps.\n";
#ifdef WIN32
        MessageBox (NULL, s.str().c_str() , "Performance", MB_OK);
#else
        std::cout << s.str();
#endif
    }
private:
    unsigned int count_;
};

static double s_nextFastForwardFrameRecorded=0; // the next frame to render in recorded time
static double s_nextFastForwardFrameReal=0;     // the next frame to render in real time
#endif // DEDICATED

// settings for fast forward mode
static REAL sr_FF_Maxstep=1; // maximum step between rendered frames
static tSettingItem<REAL> c_ff( "FAST_FORWARD_MAXSTEP",
                                sr_FF_Maxstep );

static REAL sr_FF_MaxstepReal=.05; // maximum step in real time between rendered frames
static tSettingItem<REAL> c_ffre( "FAST_FORWARD_MAXSTEP_REAL",
                                  sr_FF_MaxstepReal );

static REAL sr_FF_MaxstepRel=1; // maximum step between rendered frames relative to end of FF mode
static tSettingItem<REAL> c_ffr( "FAST_FORWARD_MAXSTEP_REL",
                                 sr_FF_MaxstepRel );


static double s_fastForwardTo=0;
static bool   s_fastForward =false;
static bool   s_benchmark   =false;

class rFastForwardCommandLineAnalyzer: public tCommandLineAnalyzer
{
private:
    bool DoAnalyze( tCommandLineParser & parser, int pass ) override
    {
        if(pass > 0)
            return false;

        // get option
        tString forward;
        if ( parser.GetOption( forward, "--fastforward" ) )
        {
            // set fast forward mode
            s_fastForward = true;

            // read time
            std::stringstream str(static_cast< char const * >( forward ) );
            str >> s_fastForwardTo;

            return true;
        }

        if ( parser.GetSwitch( "--benchmark" ) )
        {
            // set benchmark mode
            s_benchmark = true;
            return true;
        }

        return false;
    }

    void DoHelp( std::ostream & s ) override
    {                                      //
        s << "--fastforward <time>         : lets time run very fast until the given time is reached\n";
        s << "--benchmark                  : renders frames as they were recorded\n";
    }
};

static rFastForwardCommandLineAnalyzer analyzer;

rSysDep::rSwapMode rSysDep::swapMode_ = rSysDep::rSwap_glFlush;
rSysDep::OverlayRenderFunc rSysDep::overlayRenderFunc_ = NULL;
//rSysDep::rSwapMode rSysDep::swapMode_ = rSysDep::rSwap_60Hz;

int sr_maxFPS = 360;

// buffer swap:
#ifndef DEDICATED
// for setting breakpoints in optimized mode, too
static void breakpoint(){}

static bool sr_netSyncThreadGoOn = true;
static rSysDep::rNetIdler * sr_netIdler = NULL;
int sr_NetSyncThread(void *lockVoid)
{
    SDL_Mutex *lock = (SDL_Mutex *)lockVoid;

    SDL_LockMutex(lock);

    while ( sr_netSyncThreadGoOn )
    {
        SDL_UnlockMutex(lock);
        // wait for network data
        bool toDo = sr_netIdler->Wait();
        SDL_LockMutex(lock);

        if ( toDo )
        {
            // disable rendering (during auto-scrolling of console, for example)
            bool glout = sr_glOut;
            sr_glOut = false;

            // new network data arrived, handle it
            sr_netIdler->Do();

            // enable rendering again
            sr_glOut = glout;
        }
    }

    SDL_UnlockMutex(lock);

    return 0;
}

static SDL_Thread * sr_netSyncThread = NULL;
static SDL_Mutex * sr_netLock = NULL;
void rSysDep::StartNetSyncThread( rNetIdler * idler )
{
    sr_netIdler = idler;

    return;

    // can't use thrading trouble while recording
    if ( tRecorder::IsRunning() )
        return;

    if ( sr_netSyncThread )
        return;

    // create lock
    if ( !sr_netLock )
        sr_netLock = SDL_CreateMutex();

    // start thread
    // SDL3: SDL_CreateThread requires a name argument
    sr_netSyncThread = SDL_CreateThread( sr_NetSyncThread, "NetSyncThread", sr_netLock );
    if ( !sr_netSyncThread )
        return;

    // lock mutex, the thread should only do work while the main thread is waiting for the refresh
    SDL_LockMutex( sr_netLock );
}

void rSysDep::StopNetSyncThread()
{
    // stop and delete thread
    if ( sr_netSyncThread )
    {
        SDL_UnlockMutex(  sr_netLock );
        sr_netSyncThreadGoOn = false;
        SDL_WaitThread( sr_netSyncThread, NULL );
        sr_netSyncThread = NULL;
        sr_netIdler = NULL;
    }

    // delete lock
    if ( sr_netLock )
    {
        SDL_DestroyMutex( sr_netLock );
        sr_netLock = NULL;
    }
}

static tConfItem<int> sr_maxFPSConf("MAX_FPS", sr_maxFPS,
                                    [](const int& val) { return (val >= 0); });

void sr_LimitFPS()
{
    if (sr_maxFPS > 0 && !tRecorder::IsPlayingBack())
    {
        static double last_time = 0;

        const double now_time = tRealSysTimeFloat();
        const double SPF = 1.0 / sr_maxFPS;

        const double target_now_time = last_time + SPF;
        if (now_time < target_now_time)
        {
            // OPTIMIZATION: hybrid sleep+spin for sub-ms precision
            // SDL_Delay has ~1-15ms granularity on Linux, causing input lag.
            // Sleep the bulk, then spin-wait the last ~1.5ms.
            double remaining = target_now_time - now_time;
            if (remaining > 0.0015)
            {
                SDL_Delay((int)((remaining - 0.0015) * 1000));
            }
            // Spin-wait for final precision (~0-1.5ms, negligible CPU)
            while (tRealSysTimeFloat() < target_now_time)
            {
                // busy-wait for sub-millisecond accuracy
            }
            last_time = target_now_time;
        }
        else
        {
            last_time = now_time;
        }
    }
}

void rSysDep::SwapGL(){
    if ( s_benchmark )
    {
        static PerformanceCounter counter;
        counter.Count();
    }

    double time = tSysTimeFloat();
    double realTime = tRealSysTimeFloat();

    bool next_glOut = sr_glOut;

    // adapt playback speed to recorded speed
    if ( !s_benchmark && !s_fastForward && tRecorder::IsPlayingBack() )
    {
        static double timeOffset=0;
        static double lastRendered=0;

        // calculate how much we're behind the rendering schedule
        double behind = - time + realTime + timeOffset;
        // std::cout << behind << " " << sr_glOut << "\n";

        // large delays can only be caused by breakpoints or map downloads; ignore them
        if ( behind > .5 || realTime > lastRendered + .2 )
        {
            timeOffset -= behind;
            next_glOut = true;
        }
        else
        {
            // we're a bit behind, skip the next frame
            if ( behind > .1 )
            {
                next_glOut = false;
            }
            else if ( sr_glOut )
            {
                lastRendered=realTime;
                // we're ahead, pause a bit
                if  ( behind < -.5 )
                    timeOffset -= behind;
                else if ( behind < -.1 )
                {
                    int delay = int( -( behind + .1 ) * 1000000 );
                    // std::cout << behind << ":" << delay << "\n";
                    tDelayForce( delay );
                }
            }
            else
            {
                // we're not behind any more. Reactivate rendering.
                next_glOut = true;
            }
        }

        if ( next_glOut )
            lastRendered=realTime;
    }

    if (!sr_glOut)
    {
        // display next frame in fast foward mode
        if ( ( s_fastForward && ( time > s_nextFastForwardFrameRecorded || realTime > s_nextFastForwardFrameReal ) ) || next_glOut )
        {
            sr_glOut = true;
            rSysDep::ClearGL();
        }

        // in playback or recording mode, always execute frame tasks, they may be improtant for consistency
        if ( tRecorder::IsRunning() )
            rPerFrameTask::DoPerFrameTasks();

        return;
    }


    rPerFrameTask::DoPerFrameTasks();

    // Render overlay (visual menu) on top of everything
    if (overlayRenderFunc_) {
        overlayRenderFunc_();
    }

    // unlock the mutex while waiting for the swap operation to finish
    SDL_UnlockMutex(  sr_netLock );
    sr_LockSDL();

    switch( swapMode_ )
    {
    case rSwap_Fastest:
        break;
    case rSwap_glFlush:
        glFlush();
        break;
    case rSwap_glFinish:
        glFinish();
        break;
    }

#if defined(SDL_OPENGL)
    if (lastSuccess.useSDL)
        SDL_GL_SwapWindow(sr_screen);
    //#elif defined(HAVE_FXMESA)
    //fxMesaSwapBuffers();
#endif

#ifdef DIRTY
    if (!lastSuccess.useSDL){
#if defined(WIN32)
        SwapBuffers( hDC );
#elif defined(unix) || defined(__unix__)
        glXSwapBuffers(dpy,win);
#endif
    }
#endif

    if (sr_screenshotIsPlanned){
        make_screenshot();
        sr_screenshotIsPlanned=false;
    }

    sr_UnlockSDL();
    // lock mutex again
    SDL_LockMutex(  sr_netLock );


    // disable output in fast forward mode
    if ( s_fastForward && tRecorder::IsPlayingBack() )
    {
        if ( time < s_fastForwardTo )
        {
            // next displayed frame should be ten percent closer to the target, but at most 10 seconds
            s_nextFastForwardFrameRecorded = ( s_fastForwardTo - time ) * sr_FF_MaxstepRel;
            if ( s_nextFastForwardFrameRecorded > sr_FF_Maxstep )
                s_nextFastForwardFrameRecorded = sr_FF_Maxstep ;
            s_nextFastForwardFrameRecorded += time;
            s_nextFastForwardFrameReal = realTime + sr_FF_MaxstepReal ;

            next_glOut = false;
        }
        else
        {
            std::cout << "End of fast forward mode.\n";
            st_Breakpoint();
            s_fastForward = false;
        }
    }

    //#ifdef DEBUG
    if ( !s_fastForward )
    {
        breakpoint();
    }
    //#endif

    sr_LimitFPS();

    sr_glOut = next_glOut;
}
#endif // dedicated

#ifndef DEDICATED
static SDL_Mutex *mut;

static void stuff_init(){
    mut=SDL_CreateMutex();
}

static tInitExit stuff_ie(&stuff_init);
#endif

void sr_LockSDL(){
    //std::cerr << "locking...";
#ifndef DEDICATED
#ifndef WIN32
    //SDL_LockMutex(mut);
#endif
#endif
    //std::cerr << " locked!\n";
}

void sr_UnlockSDL(){
    //std::cerr << "unlocking...";
#ifndef DEDICATED
#ifndef WIN32
    //SDL_UnlockMutex(mut);
#endif
#endif
    //std::cerr << " unlocked!\n";
}

#ifndef DEDICATED
void  rSysDep::ClearGL(){
    if (sr_glOut){

        /*
        if (sr_screenshotIsPlanned){
          make_screenshot();
          sr_screenshotIsPlanned=false;
        }
        */

        glClearColor(0.0,0.0,0.0,1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}
#endif


