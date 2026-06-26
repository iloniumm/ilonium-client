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

#include "uInputQueue.h"
#include "rScreen.h"
#include "tConfiguration.h"
#include <iostream>

#ifndef DEDICATED
#include "rSDL.h"
#endif

#include  "tRecorder.h"
#include  "uMenu.h"
#include  "../tron/gMenus.h"
#include  "../tron/ModMenu.h"

static su_TimerCallback *timer=NULL;

su_TimerCallback::su_TimerCallback(){
    timer = this;
}

su_TimerCallback::~su_TimerCallback(){
    if (timer == this)
        timer = NULL;
}

static inline REAL Time(){
    if (timer)
        return timer->GetTime();
    else
        return 0;
}

bool su_prefetchInput=false;
bool su_contInput=true;

#define MAX_PENDING_INPUT 100

static REAL times[MAX_PENDING_INPUT];
static SDL_Event tEvents[MAX_PENDING_INPUT];

static int   currentIn=0,current_out=0,next_in=1;


static inline void increase(int &i){
    i++;
    if (i>=MAX_PENDING_INPUT)
        i=0;
}


static bool input_get=false;

void su_FetchAndStoreSDLInput()
{
#ifndef DEDICATED
#ifndef WIN32
#ifndef MACOSX
    if (!tRecorder::IsRunning() )
        SDL_PumpEvents();
#endif
#endif
#endif
}


bool su_StoreSDLEvent(const SDL_Event &tEvent){
    if (next_in!=current_out && !input_get){
        //con << "Extra input!\n";
#ifndef DEDICATED
        if (tEvents[currentIn].type == SDL_EVENT_TEXT_INPUT && tEvents[currentIn].text.text) {
            SDL_free((void*)tEvents[currentIn].text.text);
            tEvents[currentIn].text.text = nullptr;
        }
        if (tEvents[currentIn].type == SDL_EVENT_TEXT_EDITING && tEvents[currentIn].edit.text) {
            SDL_free((void*)tEvents[currentIn].edit.text);
            tEvents[currentIn].edit.text = nullptr;
        }
#endif

        tEvents[currentIn]=tEvent;

#ifndef DEDICATED
        if (tEvent.type == SDL_EVENT_TEXT_INPUT && tEvent.text.text) {
            tEvents[currentIn].text.text = SDL_strdup(tEvent.text.text);
        }
        if (tEvent.type == SDL_EVENT_TEXT_EDITING && tEvent.edit.text) {
            tEvents[currentIn].edit.text = SDL_strdup(tEvent.edit.text);
        }
#endif

        times[currentIn]=Time();
        increase(currentIn);
        next_in=currentIn;
        increase(next_in);
        return false;
    }
    return true;
}

#ifndef DEDICATED
// read and write operators for keysyms
tRECORDING_ENUM( SDL_Keycode );
tRECORDING_ENUM( SDL_Keymod );
tRECORDING_ENUM( SDL_Scancode );
#endif

static char const * recordingSection = "INPUT";

//! Read or write event data
template< class Archiver > class EventArchiver
{
public:
#ifndef DEDICATED
    static void ArchiveKey( Archiver & archive, SDL_KeyboardEvent & key )
    {
        archive.Archive(key.down).Archive(key.scancode).Archive(key.key).Archive(key.mod);
    }
#endif

    static bool Archive( SDL_Event & event, REAL & time, bool & ret )
    {
        // start archive block if archiving is active
        Archiver archive;
        if ( archive.Initialize( recordingSection ) )
        {
#ifndef DEDICATED
            archive.Archive( ret );
            if ( !ret )
                return false;

            // write or read data
            archive.Archive(time).Archive(event.type);
            switch ( event.type )
            {
            // SDL_ACTIVEEVENT removed in SDL3; window focus handled via SDL_EVENT_WINDOW_FOCUS_*
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
            {
                SDL_KeyboardEvent & key = event.key;
                ArchiveKey( archive, key );
            }
            break;
            case SDL_EVENT_MOUSE_MOTION:
            {
                SDL_MouseMotionEvent & motion = event.motion;

                archive.Archive(motion.state).Archive(motion.x).Archive(motion.y).Archive(motion.xrel).Archive(motion.yrel);
            }
            break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                SDL_MouseButtonEvent & button = event.button;

                archive.Archive(button.button).Archive(button.down).Archive(button.x).Archive(button.y);
            }
            break;
            default:
                // do nothing
                break;
            }

#endif  // DEDICATED

            return true;
        }

        return false;
    }
};

#ifndef DEDICATED
//! Read or write event data
template<>
void EventArchiver< tRecordingBlock >::ArchiveKey( tRecordingBlock & archive, SDL_KeyboardEvent & orig )
{
    SDL_KeyboardEvent key = orig;
    if ( uInputScrambler::Scrambled() )
    {
        switch( key.key )
        {
        case SDLK_ESCAPE:
        case SDLK_SPACE:
        case SDLK_KP_ENTER:
        case SDLK_RETURN:
        case SDLK_UP:
        case SDLK_DOWN:
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_BACKSPACE:
        case SDLK_DELETE:
            break;
        default:
            key.mod = SDL_KMOD_NONE;
            key.key = SDLK_X;
            key.scancode = SDL_SCANCODE_UNKNOWN;
            // unicode field removed in SDL3; text input handled via SDL_EVENT_TEXT_INPUT
        }
    }

    archive.Archive(key.down).Archive(key.scancode).Archive(key.key).Archive(key.mod);
}
#endif

static const char * su_end = "END";
static const char * su_endInput = "ENDINPUT";

// flag indicating input was made and an input start marker is needed for the next input loop
static bool su_markerRequired = false;

void su_EndGetSDLInput()
{
    if ( su_markerRequired )
    {
        // record end of input fetching
        tRecorder::Playback(su_endInput);
        tRecorder::Record(su_endInput);
        su_markerRequired = false;
    }
}

uInputProcessGuard::uInputProcessGuard()
{}
uInputProcessGuard::~uInputProcessGuard()
{
    su_EndGetSDLInput();
}

int uInputScrambler::scrambled_ = 0;

uInputScrambler::uInputScrambler()
{
    scrambled_ ++;
}

uInputScrambler::~uInputScrambler()
{
    --scrambled_;
}

bool uInputScrambler::Scrambled()
{
    return scrambled_ > 0;
}

bool su_GetSDLInput(SDL_Event &tEvent,REAL &time){
    // Check if there is a pending legacy menu action from the Mod Menu
    if (ModMenu::g_PendingLegacyMenuAction) {
        auto action = ModMenu::g_PendingLegacyMenuAction;
        ModMenu::g_PendingLegacyMenuAction = nullptr;

        bool wasOpen = ModMenu::IsOpen();
        bool wasInGameOpen = ModMenu::g_InGameMenuOpen;
        
        if (wasOpen) ModMenu::SetOpen(false);
        if (wasInGameOpen) ModMenu::g_InGameMenuOpen = false;

        action();

        if (wasOpen) ModMenu::SetOpen(true);
        if (wasInGameOpen) ModMenu::g_InGameMenuOpen = true;

        return false;
    }

    bool ret=false;

    // clear out data
    memset( &tEvent, 0, sizeof( SDL_Event ) );

    // find end of recording in playback
    if ( tRecorder::Playback(su_end) )
    {
        tRecorder::Record(su_end);
        uMenu::quickexit=uMenu::QuickExit_Total;
    }

    // try to fetch event from playback
    if ( !EventArchiver< tPlaybackBlock >::Archive( tEvent, time, ret ) )
    {
        // get real event
        sr_LockSDL();
        input_get=true;
        if (current_out!=currentIn){
            time=times[current_out];
            tEvent=tEvents[current_out];
            increase(current_out);
            ret=true;
        }
        else{
            time=Time();
            ret=
#ifndef DEDICATED
                SDL_PollEvent(&tEvent);
#else
                false;
#endif
        }
        sr_UnlockSDL();
        input_get=false;
    }

    su_markerRequired |= ret;


    // [MOD] Intercept for Visual Menu
    if (ret) {
        if (cVisualMenu::HandleEvent(tEvent)) {
            tEvent.type = SDL_EVENT_USER;
        }
    }

    // store event in recording
    if ( ret )
        EventArchiver< tRecordingBlock >::Archive( tEvent, time, ret );

#ifndef DEDICATED
    // filter bogus events. Some keys cause key events with wrong keysyms.
    // In SDL3, bogus-unicode filtering is no longer needed; SDL_EVENT_TEXT_INPUT
    // handles printable characters separately from SDL_EVENT_KEY_DOWN.
#endif

    return ret;
}

/*
int su_InputThread(void *){
    while (su_contInput){
        if (sr_screen && su_prefetchInput){
            sr_LockSDL();
#ifndef DEDICATED
            SDL_PumpEvents();
#endif
            sr_UnlockSDL();
        }
#ifndef WIN32
        usleep(100000);
#endif
    }
    return 0;
}
*/


