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

#include "uInput.h"
#include "tMemManager.h"
#include "rScreen.h"
#include "tInitExit.h"
#include "tConfiguration.h"
#include "rConsole.h"
#include "uMenu.h"
#include "tSysTime.h"

bool su_mouseGrab = false;

static uAction* su_allActions[uMAX_ACTIONS];
static int     su_allActionsLen = 0;

uAction::uAction(uAction *&anchor,const char* name,
                 int priority_,
                 uInputType t)
        :tListItem<uAction>(anchor),tooltip_(NULL),type(t),priority(priority_),internalName(name){
    globalID = localID = su_allActionsLen++;

    tASSERT(localID < uMAX_ACTIONS);

    su_allActions[localID] = this;

    tString descname;
    descname << "input_" << name << "_text";
    tToLower( descname );

    const_cast<tOutput&>(description).AddLocale(descname);

    tString helpname;
    helpname << "input_" << name << "_help";
    tToLower( helpname );

    const_cast<tOutput&>(helpText).AddLocale(helpname);
}

uAction::uAction(uAction *&anchor,const char* name,
                 const tOutput& desc,
                 const tOutput& help,
                 int priority_,
                 uInputType t)
        :tListItem<uAction>(anchor),tooltip_(NULL),type(t),priority(priority_),internalName(name), description(desc), helpText(help){
    globalID = localID = su_allActionsLen++;

    tASSERT(localID < uMAX_ACTIONS);

    su_allActions[localID] = this;
}

uAction::~uAction(){
    su_allActions[localID] = NULL;
}

uAction * uAction::Find( char const * name )
{
    for(int i=su_allActionsLen-1;i>=0;i--)
        if(!strcmp(name,su_allActions[i]->internalName))
            return su_allActions[i];

    return 0;
}

static int QWERTYKeycodeToScancode(int keycode) {
    if (keycode >= 'a' && keycode <= 'z') {
        return SDL_SCANCODE_A + (keycode - 'a');
    }
    if (keycode >= 'A' && keycode <= 'Z') {
        return SDL_SCANCODE_A + (keycode - 'A');
    }
    if (keycode >= '1' && keycode <= '9') {
        return SDL_SCANCODE_1 + (keycode - '1');
    }
    if (keycode == '0') {
        return SDL_SCANCODE_0;
    }
    switch (keycode) {
        case '-':  return SDL_SCANCODE_MINUS;
        case '=':  return SDL_SCANCODE_EQUALS;
        case '[':  return SDL_SCANCODE_LEFTBRACKET;
        case ']':  return SDL_SCANCODE_RIGHTBRACKET;
        case '\\': return SDL_SCANCODE_BACKSLASH;
        case ';':  return SDL_SCANCODE_SEMICOLON;
        case '\'': return SDL_SCANCODE_APOSTROPHE;
        case '`':  return SDL_SCANCODE_GRAVE;
        case ',':  return SDL_SCANCODE_COMMA;
        case '.':  return SDL_SCANCODE_PERIOD;
        case '/':  return SDL_SCANCODE_SLASH;
    }
    return SDL_SCANCODE_UNKNOWN;
}

static int ScancodeToQWERTYKeycode(int scancode) {
    if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) {
        return 'a' + (scancode - SDL_SCANCODE_A);
    }
    if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9) {
        return '1' + (scancode - SDL_SCANCODE_1);
    }
    switch (scancode) {
        case SDL_SCANCODE_0:            return '0';
        case SDL_SCANCODE_MINUS:        return '-';
        case SDL_SCANCODE_EQUALS:       return '=';
        case SDL_SCANCODE_LEFTBRACKET:  return '[';
        case SDL_SCANCODE_RIGHTBRACKET: return ']';
        case SDL_SCANCODE_BACKSLASH:    return '\\';
        case SDL_SCANCODE_SEMICOLON:    return ';';
        case SDL_SCANCODE_APOSTROPHE:   return '\'';
        case SDL_SCANCODE_GRAVE:        return '`';
        case SDL_SCANCODE_COMMA:        return ',';
        case SDL_SCANCODE_PERIOD:       return '.';
        case SDL_SCANCODE_SLASH:        return '/';
    }
    return 0;
}

static int TranslateLegacyKeycode(int keysym) {
    if (keysym < 0) return keysym;
    if (keysym >= 324 && keysym <= 337) {
        return keysym - 323 + 512;
    }
    if (keysym > 512) {
        return keysym;
    }
    switch (keysym) {
        case 8:   return SDL_SCANCODE_BACKSPACE;
        case 9:   return SDL_SCANCODE_TAB;
        case 13:  return SDL_SCANCODE_RETURN;
        case 27:  return SDL_SCANCODE_ESCAPE;
        case 32:  return SDL_SCANCODE_SPACE;
        case 127: return SDL_SCANCODE_DELETE;
        case 273: return SDL_SCANCODE_UP;
        case 274: return SDL_SCANCODE_DOWN;
        case 275: return SDL_SCANCODE_RIGHT;
        case 276: return SDL_SCANCODE_LEFT;
        case 277: return SDL_SCANCODE_INSERT;
        case 278: return SDL_SCANCODE_HOME;
        case 279: return SDL_SCANCODE_END;
        case 280: return SDL_SCANCODE_PAGEUP;
        case 281: return SDL_SCANCODE_PAGEDOWN;
        case 282: return SDL_SCANCODE_F1;
        case 283: return SDL_SCANCODE_F2;
        case 284: return SDL_SCANCODE_F3;
        case 285: return SDL_SCANCODE_F4;
        case 286: return SDL_SCANCODE_F5;
        case 287: return SDL_SCANCODE_F6;
        case 288: return SDL_SCANCODE_F7;
        case 289: return SDL_SCANCODE_F8;
        case 290: return SDL_SCANCODE_F9;
        case 291: return SDL_SCANCODE_F10;
        case 292: return SDL_SCANCODE_F11;
        case 293: return SDL_SCANCODE_F12;
        case 256: return SDL_SCANCODE_KP_0;
        case 257: return SDL_SCANCODE_KP_1;
        case 258: return SDL_SCANCODE_KP_2;
        case 259: return SDL_SCANCODE_KP_3;
        case 260: return SDL_SCANCODE_KP_4;
        case 261: return SDL_SCANCODE_KP_5;
        case 262: return SDL_SCANCODE_KP_6;
        case 263: return SDL_SCANCODE_KP_7;
        case 264: return SDL_SCANCODE_KP_8;
        case 265: return SDL_SCANCODE_KP_9;
        case 266: return SDL_SCANCODE_KP_PERIOD;
        case 267: return SDL_SCANCODE_KP_DIVIDE;
        case 268: return SDL_SCANCODE_KP_MULTIPLY;
        case 269: return SDL_SCANCODE_KP_MINUS;
        case 270: return SDL_SCANCODE_KP_PLUS;
        case 271: return SDL_SCANCODE_KP_ENTER;
        case 272: return SDL_SCANCODE_KP_EQUALS;
        case 300: return SDL_SCANCODE_NUMLOCKCLEAR;
        case 301: return SDL_SCANCODE_CAPSLOCK;
        case 302: return SDL_SCANCODE_SCROLLLOCK;
        case 303: return SDL_SCANCODE_RSHIFT;
        case 304: return SDL_SCANCODE_LSHIFT;
        case 305: return SDL_SCANCODE_RCTRL;
        case 306: return SDL_SCANCODE_LCTRL;
        case 307: return SDL_SCANCODE_RALT;
        case 308: return SDL_SCANCODE_LALT;
        case 309: return SDL_SCANCODE_RGUI;
        case 310: return SDL_SCANCODE_LGUI;
        case 311: return SDL_SCANCODE_LGUI;
        case 312: return SDL_SCANCODE_RGUI;
        case 316: return SDL_SCANCODE_PRINTSCREEN;
        case 19:  return SDL_SCANCODE_PAUSE;
    }
    if (keysym < 128) {
        int sc = QWERTYKeycodeToScancode(keysym);
        if (sc != SDL_SCANCODE_UNKNOWN) {
            return sc;
        }
        SDL_Scancode sdlSc = SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(keysym));
        if (sdlSc != SDL_SCANCODE_UNKNOWN) {
            return sdlSc;
        }
    }
    return keysym;
}

static int TranslateScancodeToLegacyKeycode(int scancode) {
    if (scancode < 0) return scancode;
    
    // In TranslateLegacyKeycode:
    // if (keysym >= 324 && keysym <= 337) return keysym - 323 + 512;
    // scancode = keysym - 323 + 512 => keysym = scancode - 189
    if (scancode >= 513 && scancode <= 526) {
        return scancode - 189;
    }
    
    if (scancode >= 512) {
        return scancode;
    }

    switch (scancode) {
        case SDL_SCANCODE_BACKSPACE:      return 8;
        case SDL_SCANCODE_TAB:            return 9;
        case SDL_SCANCODE_RETURN:         return 13;
        case SDL_SCANCODE_ESCAPE:         return 27;
        case SDL_SCANCODE_SPACE:          return 32;
        case SDL_SCANCODE_DELETE:         return 127;
        case SDL_SCANCODE_UP:             return 273;
        case SDL_SCANCODE_DOWN:           return 274;
        case SDL_SCANCODE_RIGHT:          return 275;
        case SDL_SCANCODE_LEFT:           return 276;
        case SDL_SCANCODE_INSERT:         return 277;
        case SDL_SCANCODE_HOME:           return 278;
        case SDL_SCANCODE_END:            return 279;
        case SDL_SCANCODE_PAGEUP:         return 280;
        case SDL_SCANCODE_PAGEDOWN:       return 281;
        case SDL_SCANCODE_F1:             return 282;
        case SDL_SCANCODE_F2:             return 283;
        case SDL_SCANCODE_F3:             return 284;
        case SDL_SCANCODE_F4:             return 285;
        case SDL_SCANCODE_F5:             return 286;
        case SDL_SCANCODE_F6:             return 287;
        case SDL_SCANCODE_F7:             return 288;
        case SDL_SCANCODE_F8:             return 289;
        case SDL_SCANCODE_F9:             return 290;
        case SDL_SCANCODE_F10:            return 291;
        case SDL_SCANCODE_F11:            return 292;
        case SDL_SCANCODE_F12:            return 293;
        case SDL_SCANCODE_KP_0:           return 256;
        case SDL_SCANCODE_KP_1:           return 257;
        case SDL_SCANCODE_KP_2:           return 258;
        case SDL_SCANCODE_KP_3:           return 259;
        case SDL_SCANCODE_KP_4:           return 260;
        case SDL_SCANCODE_KP_5:           return 261;
        case SDL_SCANCODE_KP_6:           return 262;
        case SDL_SCANCODE_KP_7:           return 263;
        case SDL_SCANCODE_KP_8:           return 264;
        case SDL_SCANCODE_KP_9:           return 265;
        case SDL_SCANCODE_KP_PERIOD:      return 266;
        case SDL_SCANCODE_KP_DIVIDE:      return 267;
        case SDL_SCANCODE_KP_MULTIPLY:    return 268;
        case SDL_SCANCODE_KP_MINUS:       return 269;
        case SDL_SCANCODE_KP_PLUS:        return 270;
        case SDL_SCANCODE_KP_ENTER:       return 271;
        case SDL_SCANCODE_KP_EQUALS:      return 272;
        case SDL_SCANCODE_NUMLOCKCLEAR:   return 300;
        case SDL_SCANCODE_CAPSLOCK:       return 301;
        case SDL_SCANCODE_SCROLLLOCK:     return 302;
        case SDL_SCANCODE_RSHIFT:         return 303;
        case SDL_SCANCODE_LSHIFT:         return 304;
        case SDL_SCANCODE_RCTRL:          return 305;
        case SDL_SCANCODE_LCTRL:          return 306;
        case SDL_SCANCODE_RALT:           return 307;
        case SDL_SCANCODE_LALT:           return 308;
        case SDL_SCANCODE_RGUI:           return 309;
        case SDL_SCANCODE_LGUI:           return 310;
        case SDL_SCANCODE_PRINTSCREEN:    return 316;
        case SDL_SCANCODE_PAUSE:          return 19;
    }

    int key = ScancodeToQWERTYKeycode(scancode);
    if (key > 0) {
        return key;
    }

    SDL_Keycode sdlKey = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(scancode), SDL_KMOD_NONE, false);
    if (sdlKey > 0 && sdlKey < 128) {
        return sdlKey;
    }

    return scancode;
}

// ****************************************
// a configuration class for keyboard binds
// ****************************************

class tConfItem_key:public tConfItemBase{
public:
    tConfItem_key():tConfItemBase("KEYBOARD"){}
    ~tConfItem_key(){}

    // write the complete keymap
    virtual void WriteVal(std::ostream &s){
        int first=1;
        for(int keysym=SDLK_NEWLAST-1;keysym>=0;keysym--){
            if (keymap[keysym]){

                if (!first)
                    s << "\nKEYBOARD\t";
                else
                    first=0;

                int legacyKeysym = TranslateScancodeToLegacyKeycode(keysym);
                s << legacyKeysym << '\t';
                keymap[keysym]->Write(s);
            }
        }
        if (first)
            s << "-1";
    }

    // read one keybind
    virtual void ReadVal(std::istream &s){
        tString in;
        int keysym;
        s >> keysym;
        keysym = TranslateLegacyKeycode(keysym);
        if (keysym>=0){
            tASSERT(keysym < SDLK_NEWLAST);
            s >> in;
            if (uBindPlayer::IsKeyWord(in))
            {
                keymap[keysym] = NULL;
                keymap[keysym]=uBindPlayer::NewBind(s);
                if (!keymap[keysym]->act)
                {
                    keymap[keysym]=NULL;
                }
                /* if (global_bind::IsKeyWord(in))
                   keymap[keysym]=new global_bind(s); */
            }
        }
        char c=' ';
        while(c!='\n' && s.good() && !s.eof()) c=s.get();
    }
};

// we need just one
static tConfItem_key x;

static uAction *s_playerActions;
static uAction *s_cameraActions;
static uAction *s_globalActions;

uActionPlayer::uActionPlayer(const char *name,
                             int priority,
                             uInputType t)
    :uAction(s_playerActions,name,priority,t){}

uActionPlayer::uActionPlayer(const char *name,
                             const tOutput& desc,
                             const tOutput& help,
                             int priority,
                             uInputType t)
        :uAction(s_playerActions,name,desc,help,priority,t){}

uActionPlayer::~uActionPlayer(){}

bool uActionPlayer::operator==(const uActionPlayer &x){
    return x.globalID == globalID;}

uActionPlayer *uActionPlayer::Find(int id){
    uAction *run = s_playerActions;

    while (run){
        if (run->ID() == id)
            return static_cast<uActionPlayer*>(run);
        run = run->Next();
    }

    return NULL;
}


uActionCamera::uActionCamera(const char *name,
                             int priority,
                             uInputType t)
    :uAction(s_cameraActions,name,priority,t){}

uActionCamera::uActionCamera(const char *name,
                             const tOutput& desc,
                             const tOutput& help,
                             int priority,
                             uInputType t)
    :uAction(s_cameraActions,name,desc,help,priority,t){}

uActionCamera::~uActionCamera(){}

bool uActionCamera::operator==(const uActionCamera &x){
    return x.globalID == globalID;}


// global actions
uActionGlobal::uActionGlobal(const char *name,
                             int priority,
                             uInputType t)
        :uAction(s_globalActions,name,priority,t){}

uActionGlobal::~uActionGlobal(){}

bool uActionGlobal::operator==(const uActionGlobal &x){
    return x.globalID == globalID;}

bool uActionGlobal::IsBreakingGlobalBind(int sym){
    if (sym < 0 || sym >= SDLK_NEWLAST)
        return false;
    if (!keymap[sym])
        return false;
    uAction *act=keymap[sym]->act;
    if (!act)
        return false;

    return uActionGlobalFunc::IsBreakingGlobalBind(act);
}

// ***************************
//    the generic keymaps
// ***************************

tJUST_CONTROLLED_PTR< uBind > keymap[SDLK_NEWLAST];
bool                          pressed[SDLK_NEWLAST];

static void keyboard_init(){
    for(int i=0;i<SDLK_NEWLAST;i++)
        keymap[i]=NULL;
    //uBindPlayer::Init();
    //global_bind::Init();
}

static void keyboard_exit(){
    for(int i=0;i<SDLK_NEWLAST;i++)
        keymap[i] = 0;
    //uBindPlayer::Init();
    //global_bind::Init();
}

static tInitExit keyboard_ie(&keyboard_init, &keyboard_exit);

// *********************************************
// generic keypress/mouse movement binding class
// *********************************************

uBind::~uBind(){}

uBind::uBind(uAction *a ):lastValue_(0), delayedValue_(0), lastSym_(-1), lastTime_(-1), act(a){}

uBind::uBind(std::istream &s): lastValue_(0), delayedValue_(0), lastSym_(-1), lastTime_(-1), act(NULL)
{
    std::string name;
    s >> name;
    act = uAction::Find( name.c_str() );
}

void uBind::Write(std::ostream &s){
    s << act->internalName << '\t';
}

bool GlobalAct(uAction *act,REAL x){
    return uActionGlobalFunc::GlobalAct(act,x);
}

bool uBind::Activate(REAL x, bool delayed )
{
    delayedValue_ = x;

    if ( !delayed || !Delayable() )
    {
        lastValue_ = x;
        return this->DoActivate( x );
    }

    return true;
}

void uBind::HanldeDelayed()
{
    if ( lastValue_ != delayedValue_ )
    {
        lastValue_ = delayedValue_;
        this->DoActivate( delayedValue_ );
    }
}

REAL su_doubleBindTimeout=-10.0f;

bool uBind::IsDoubleBind( int sym )
{
    double currentTime = tSysTimeFloat();

    // if a different key was used for this action a short while ago, give alarm.
    bool ret = ( su_doubleBindTimeout > 0 && sym != lastSym_ && currentTime - lastTime_ < su_doubleBindTimeout );

    // store last usage
    lastSym_ = sym;
    lastTime_ = currentTime;

    // return result
    return ret;
}

// *******************
// player config
// *******************

static int nextid = 0;

uPlayerPrototype* uPlayerPrototype::PlayerConfig(int i){
    tASSERT(i>=0 && i<uMAX_PLAYERS);
    return playerConfig[i];
}


uPlayerPrototype::uPlayerPrototype(){
    static bool inited=false;
    if (!inited)
    {
        for(int i=uMAX_PLAYERS-1; i >=0; i--)
            playerConfig[i] = NULL;

        inited = true;
    }

    id = nextid++;
    tASSERT(id < uMAX_PLAYERS);
    playerConfig[id] = this;


}

uPlayerPrototype::~uPlayerPrototype(){
    playerConfig[id] = NULL;
}

uPlayerPrototype* uPlayerPrototype::playerConfig[uMAX_PLAYERS];

int uPlayerPrototype::Num(){return nextid;}

// *******************
// Input configuration
// *******************


// *****************************************************
//  Menuitem for input selection
// *****************************************************

static char const * keyname(int sym){
#ifndef DEDICATED
    if (sym<=SDLK_LAST)
        return SDL_GetScancodeName(static_cast<SDL_Scancode>(sym));
    else switch (sym){
        case SDLK_MOUSE_X_PLUS: return "Mouse right";
        case SDLK_MOUSE_X_MINUS: return "Mouse left";
        case SDLK_MOUSE_Y_PLUS: return "Mouse up";
        case SDLK_MOUSE_Y_MINUS: return "Mouse down";
        case SDLK_MOUSE_Z_PLUS: return "Mouse z up";
        case SDLK_MOUSE_Z_MINUS: return "Mouse z down";
        case SDLK_MOUSE_BUTTON_1: return "Mousebutton 1";
        case SDLK_MOUSE_BUTTON_2: return "Mousebutton 2";
        case SDLK_MOUSE_BUTTON_3: return "Mousebutton 3";
        case SDLK_MOUSE_BUTTON_4: return "Mousebutton 4";
        case SDLK_MOUSE_BUTTON_5: return "Mousebutton 5";
        case SDLK_MOUSE_BUTTON_6: return "Mousebutton 6";
        case SDLK_MOUSE_BUTTON_7: return "Mousebutton 7";
        }
#endif
    return "";
}

class uMenuItemInput: uMenuItem{
    uAction      *act;
    int         ePlayer;
    bool        active;
public:
    uMenuItemInput(uMenu *M,uAction *a,int p)
            :uMenuItem(M,a->helpText),act(a),ePlayer(p),active(0){
    }

    virtual ~uMenuItemInput(){}

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0){
        DisplayText(REAL(x-.02),y,act->description,selected,alpha,1);

        if (active)
        {
            tString s;
            s << tOutput("$input_press_any_key");
            DisplayText(REAL(x+.02),y,s,selected,alpha,-1);
        }
        else{
            tString s;

            bool first=1;

            for(int keysym=SDLK_NEWLAST-1;keysym>=0;keysym--)
                if(keymap[keysym] &&
                        keymap[keysym]->act==act &&
                        keymap[keysym]->CheckPlayer(ePlayer)){
                    if (!first)
                        s << ", ";
                    else
                        first=0;

                    s << keyname(keysym);
                }
            if(!first)
                DisplayText(REAL(x+.02),y,s,selected,alpha,-1);
            else
                DisplayText(REAL(x+.02),y,tOutput("$input_items_unbound"),selected,alpha,-1);
        }
    }

    virtual void Enter(){
        active=1;
    }

#define MTHRESH 5
#define MREL    2

#ifndef DEDICATED

    virtual bool Event(SDL_Event &e){
        int sym=-1;
        switch (e.type){
        case SDL_EVENT_MOUSE_MOTION:
            if(active){
                REAL xrel=e.motion.xrel;
                REAL yrel=-e.motion.yrel;

                if (fabs(xrel)>MREL*fabs(yrel)){ // x motion
                    if (xrel>MTHRESH) // left
                        sym=SDLK_MOUSE_X_PLUS;
                    if (xrel<-MTHRESH) // left
                        sym=SDLK_MOUSE_X_MINUS;
                }

                if (fabs(yrel)>MREL*fabs(xrel)){ // x motion
                    if (yrel>MTHRESH) // left
                        sym=SDLK_MOUSE_Y_PLUS;
                    if (yrel<-MTHRESH) // left
                        sym=SDLK_MOUSE_Y_MINUS;
                }

                if (sym>0)
                    active=0;
            }

            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if(active){
                int button=e.button.button;
                if (button<=MOUSE_BUTTONS)
                    sym=SDLK_MOUSE_BUTTON_1+button-1;

                active=0;
            }
            break;

        case SDL_EVENT_KEY_DOWN:{
                if(!active){
                    if (e.key.key==SDLK_DELETE || e.key.key==SDLK_BACKSPACE)
                    {
                        for(int keysym=SDLK_NEWLAST-1;keysym>=0;keysym--)
                            if(keymap[keysym] &&
                                    keymap[keysym]->act==act &&
                                    keymap[keysym]->CheckPlayer(ePlayer)){
                                keymap[keysym]=NULL;
                            }
                        return true;
                    }
                    return false;
                }

                active=0;

                if (e.key.key!=SDLK_ESCAPE)
                    sym=e.key.scancode;
                else
                    return true;
            }
            break;
        default:
            return(false);
        }

        if(sym>=0){
            if(keymap[sym] &&
                    keymap[sym]->act==act &&
                    keymap[sym]->CheckPlayer(ePlayer)){
                keymap[sym]=NULL;
            }
            else{
                keymap[sym]=NULL;
                keymap[sym]=uBindPlayer::NewBind(act,ePlayer);
            }
            return true;
        }
        return false;
    }
#endif

    virtual tString Help(){
        tString ret;
        ret << helpText << "\n";
        ret << tOutput("$input_item_help");
        return ret;
    }
};

namespace
{
class Input_Comparator
{
public:
    static int Compare( const uAction* a, const uAction* b )
    {
        if ( a->priority < b->priority )
            return 1;
        else if ( a->priority > b->priority )
            return -1;
        return tString::CompareAlphaNumerical( a->internalName, b->internalName );
    }
};
}

static void s_InputConfigGeneric(int ePlayer, uAction *&actions,const tOutput &title){
    uMenuItemInput **input;

    uMenu input_menu(title);

    uActionTooltip::Disable(ePlayer+1);

    uAction::Sort<Input_Comparator>(actions);

    int len = uAction::Len(actions);

    input=tNEW(uMenuItemInput*)[len];
    int a=0;
    for(uAction *A=actions;A; A = A->Next()){
        input[a++]=new uMenuItemInput(&input_menu,
                                      A,
                                      ePlayer+1);

    }

    input_menu.ReverseItems();
    input_menu.Enter();

    for(int b=a-1;b>=0;b--)
        delete input[b];
    delete[] input;
}

void su_InputConfig(int ePlayer){

    tOutput name;
    name.SetTemplateParameter(1, ePlayer+1);
    name.SetTemplateParameter(2, uPlayerPrototype::PlayerConfig(ePlayer)->Name());
    name << "$input_for_player";

    s_InputConfigGeneric(ePlayer,s_playerActions,name);
}

void su_InputConfigCamera(int player){

    tOutput name;
    name.SetTemplateParameter(1, uPlayerPrototype::PlayerConfig(player)->Name());
    name << "$camera_controls";

    s_InputConfigGeneric(player,s_cameraActions,name);
}

void su_InputConfigGlobal(){
    s_InputConfigGeneric(-1,s_globalActions,"$input_items_global");
}


REAL mouse_sensitivity=REAL(.1);
REAL key_sensitivity=40;
static double lastTime=0;
static REAL ts=0;

static bool su_delayed = false;

void su_HandleDelayedEvents ()
{
    // nothing to do
    if ( !su_delayed )
    {
        return;
    }

    su_delayed = false;

    for ( int i = SDLK_NEWLAST - 1; i>=0; --i )
    {
        if ( keymap[i] )
        {
            keymap[i]->HanldeDelayed();
        }
    }
}

extern int sg_noclipMouseWheel;

bool su_HandleEvent(SDL_Event &e, bool delayed ){
#ifndef DEDICATED
    int sym=-1;
    REAL pm=0;

    if ( su_delayed && !delayed )
    {
        su_HandleDelayedEvents();
    }

    su_delayed = delayed;

    // [MOD] Intercept raw mouse wheel for Noclip
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (e.button.button == 4) sg_noclipMouseWheel += 1;
        if (e.button.button == 5) sg_noclipMouseWheel -= 1;
    }

    // there is nearly allways a mouse motion tEvent:
    int xrel=e.motion.xrel;
    int yrel=-e.motion.yrel;


    switch (e.type){
    case SDL_EVENT_MOUSE_MOTION:
        if ( !su_mouseGrab || xrel != 0 || yrel != 0 )
        {
            if (keymap[SDLK_MOUSE_X_PLUS])
                keymap[SDLK_MOUSE_X_PLUS]->Activate(xrel*mouse_sensitivity, delayed );

            if (keymap[SDLK_MOUSE_X_MINUS])
                keymap[SDLK_MOUSE_X_MINUS]->Activate(-xrel*mouse_sensitivity, delayed );

            if (keymap[SDLK_MOUSE_Y_PLUS])
                keymap[SDLK_MOUSE_Y_PLUS]->Activate(yrel*mouse_sensitivity, delayed );

            if (keymap[SDLK_MOUSE_Y_MINUS])
                keymap[SDLK_MOUSE_Y_MINUS]->Activate(-yrel*mouse_sensitivity, delayed );
        }


        return true; // no fuss: allways pretend to have handled this.
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:{
            int button=e.button.button;
            if (button<=MOUSE_BUTTONS){
                sym=SDLK_MOUSE_BUTTON_1+button-1;
            }
        }
        if (e.type==SDL_EVENT_MOUSE_BUTTON_DOWN)
            pm=1;
        else
            pm=-1;
        break;

    case SDL_EVENT_KEY_DOWN:
        sym=e.key.scancode;
        pm=1;
        break;

    case SDL_EVENT_KEY_UP:
        sym=e.key.scancode;
        pm=-1;
        break;

    default:
        break;
    }
    if (sym>=0 && sym<SDLK_NEWLAST && keymap[sym]){
        REAL realpm=pm;
        if (keymap[sym]->act->type==uAction::uINPUT_ANALOG)
            pm*=ts*key_sensitivity;
        pressed[sym]=(realpm>0);
        if ( pm > 0 && keymap[sym]->IsDoubleBind( sym ) )
            return true;
        return (keymap[sym]->Activate(pm, delayed ));

    }
    else
#endif  
        return false;
}

void su_InputSync(){
    double time=tSysTimeFloat();
    ts=REAL(time-lastTime);

    //static REAL tsSmooth=0;
    //tsSmooth+=REAL(ts*.1);
    //tsSmooth/=REAL(1.1);
    lastTime=time;

    for(int sym=SDLK_NEWLAST-1;sym>=0;sym--)
        if (pressed[sym] && keymap[sym] &&
                keymap[sym]->act->type==uAction::uINPUT_ANALOG)
            keymap[sym]->Activate(ts*key_sensitivity, su_delayed );
}

void su_ClearKeys()
{
    for(int sym=SDLK_NEWLAST-1;sym>=0;sym--)
    {
        if (pressed[sym] && keymap[sym] )
            keymap[sym]->Activate(-1, su_delayed );
        pressed[sym] = false;
    }
}

// *****************
// Player binds
// *****************

static char const * Player_keyword="PLAYER_BIND";

uBindPlayer::uBindPlayer(uAction *a,int p):uBind(a),ePlayer(p){}

uBindPlayer::~uBindPlayer(){}

uBindPlayer * uBindPlayer::NewBind(std::istream &s)
{
    // read action
    std::string actionName;
    s >> actionName;
    uAction * act = uAction::Find( actionName.c_str() );

    // read player ID
    int player;
    s >> player;

    // delegate
    return NewBind( act, player );
}

uBindPlayer * uBindPlayer::NewBind( uAction * action, int player )
{
    // see if the bind has an alias
    for ( int i = SDLK_NEWLAST-1; i >= 0; --i )
    {
        // compare action
        uBind * old = keymap[i];
        if ( old && old->act == action )
        {
            uBindPlayer * oldPlayer = dynamic_cast< uBindPlayer * >( old );
            if ( oldPlayer && oldPlayer->ePlayer == player )
                return oldPlayer;
        }
    }

    // no alias found, return new bind
    return tNEW(uBindPlayer)( action, player );
}

bool uBindPlayer::IsKeyWord(const char *n){
    return !strcmp(n,Player_keyword);
}

bool uBindPlayer::CheckPlayer(int p){
    return p==ePlayer;
}

void uBindPlayer::Write(std::ostream &s){
    s << Player_keyword << '\t';
    uBind::Write(s);
    s << ePlayer;
}

bool uBindPlayer::Delayable()
{
    return ( ePlayer!=0 );
}

bool uBindPlayer::DoActivate(REAL x){
    bool ret = false;
    if (ePlayer==0)
        ret = GlobalAct(act,x);
    else
        ret = uPlayerPrototype::PlayerConfig(ePlayer-1)->Act(act,x);

    if( ret && act && act->GetTooltip() && x > 0 )
    {
        act->GetTooltip()->Count(ePlayer);
    }
    
    return ret;
}


// *****************
// Global actions
// *****************

static uActionGlobalFunc *uActionGlobal_anchor=NULL;

uActionGlobalFunc::uActionGlobalFunc(uActionGlobal *a, ACTION_FUNC *f,
                                     bool rebind )
        :tListItem<uActionGlobalFunc>(uActionGlobal_anchor), func (f), act(a),
rebindable(rebind){}

bool uActionGlobalFunc::IsBreakingGlobalBind(uAction *act){
    for(uActionGlobalFunc *run = uActionGlobal_anchor; run ; run = run->Next())
        if (run->act == act && !run->rebindable)
            return true;

    return false;
}

bool uActionGlobalFunc::GlobalAct(uAction *act, REAL x){
    for(uActionGlobalFunc *run = uActionGlobal_anchor; run ; run = run->Next())
        if (run->act == act && run->func(x))
            return true;

    return false;
}

static uActionGlobal mess_up("MESS_UP",1);

static uActionGlobal mess_down("MESS_DOWN",2);

static uActionGlobal mess_end("MESS_END",3);

static bool messup_func(REAL x){
    if (x>0){
        sr_con.Scroll(-1);
    }
    return true;
}

static bool messdown_func(REAL x){
    if (x>0){
        sr_con.Scroll(1);
    }
    return true;
}

static bool messend_func(REAL x){
    if (x>0){
        sr_con.End(2);
    }
    return true;
}

static uActionGlobalFunc mu(&mess_up,&messup_func);
static uActionGlobalFunc md(&mess_down,&messdown_func);
static uActionGlobalFunc me(&mess_end,&messend_func);

// ********
// tooltips
// ********

uActionTooltip::uActionTooltip( uAction & action, int numHelp, VETOFUNC * veto )
: tConfItemBase(action.internalName + "_TOOLTIP"), action_( action ), veto_(veto)
{
    help_ = tString("$input_") + action.internalName + "_tooltip";
    tToLower( help_ );

    // initialize array holding the number of help attempts to give left
    for( int i = uMAX_PLAYERS; i >= 0; --i )
    {
        activationsLeft_[i] = 0; // numHelp;
    }

    action.tooltip_ = this;
}

uActionTooltip::~uActionTooltip()
{
    if( action_.tooltip_ == this )
        action_.tooltip_ = NULL;
        
}

bool uActionTooltip::Help( int player )
{
    if(player < 0 || player > uMAX_PLAYERS)
        return false;

    // find most needed tooltip
    uActionTooltip * mostWanted{};

    // keys bound to the action of the tooltip that needs help
    tString maps;
    tString last;

    // run through binds
    for( int i = SDLK_NEWLAST - 1; i >= 0; --i )
    {
        uBind * bind = keymap[i];
        if( !bind ||!bind->CheckPlayer(player) )
            continue;
        uAction * action = bind->act;
        if( !action )
            continue;
        uActionTooltip * tooltip = action->GetTooltip();
        if( !tooltip || ( tooltip->veto_ && (*tooltip->veto_)(player) ) )
        {
            continue;
        }
        
        int activationsLeft = tooltip->activationsLeft_[player];
        if( activationsLeft > 0 && 
            ( !mostWanted || mostWanted->activationsLeft_[player] < activationsLeft ) )
        {
            mostWanted = tooltip;
            maps = "";
            last = "";
        }

        // build up key list
        if( mostWanted == tooltip )
        {
            if ( maps.Len() > 1 )
            {
                maps << ", ";
            }
            if ( last.Len() > 1 )
            {
                maps << last;
            }
            last = tString("<") + keyname(i) + ">";
        }
    }

    if( mostWanted )
    {
        // notice repeats, hint at how to silence them
        {
            static uActionTooltip * lastMostWanted{};
            static int identicalTooltipCount{};

            if(mostWanted == lastMostWanted)
            {
                identicalTooltipCount++;
                if(identicalTooltipCount >= 3)
                {
                    identicalTooltipCount-=2;
                    con.CenterDisplay(tString(tOutput("$tooltip_how_to_get_rid_of")));
                    return true;
                }
            }
            else
            {
                identicalTooltipCount = 0;
            }

            lastMostWanted = mostWanted;
        }


        if( last.Len() > 1 )
        {
            if( maps.Len() > 1 )
                maps << " " << tOutput("$input_or") << " " << last;
            else
                maps = last;
        }

        con.CenterDisplay(tString(tOutput(mostWanted->help_, maps)));

        return true;
    }

    return false;
}

void uActionTooltip::Disable(int player)
{
    if(player < 0 || player > uMAX_PLAYERS)
        return;

    // run through binds
    for( int i = SDLK_NEWLAST - 1; i >= 0; --i )
    {
        uBind * bind = keymap[i];
        if( !bind ||!bind->CheckPlayer(player) )
            continue;
        uAction * action = bind->act;
        if( !action )
            continue;
        uActionTooltip * tooltip = action->GetTooltip();
        if( !tooltip )
        {
            continue;
        }

        tooltip->activationsLeft_[player] = 0;
    }
}

void uActionTooltip::Count( int player )
{
    if ( activationsLeft_[player] > 0 )
    {
        activationsLeft_[player]--;
        Help(player);
    }
}

void uActionTooltip::WriteVal(std::ostream & s )
{
    for( int i = 0; i <= uMAX_PLAYERS; ++i )
    {
        s << activationsLeft_[i] << " ";
    }
}

void uActionTooltip::ReadVal(std::istream & s )
{
    for( int i = 0; i <= uMAX_PLAYERS; ++i )
    {
        s >> activationsLeft_[i];
    }
}

bool su_IsActionActive(uAction* act, int player) {
    if (!act) return false;
    for (int sym = 0; sym < SDLK_NEWLAST; sym++) {
        if (pressed[sym] && keymap[sym] && keymap[sym]->act == act) {
            if (player < 0 || keymap[sym]->CheckPlayer(player)) {
                return true;
            }
        }
    }
    return false;
}

bool su_HasAnyBind(uAction* act, int player) {
    if (!act) return false;
    for (int sym = 0; sym < SDLK_NEWLAST; sym++) {
        if (keymap[sym] && keymap[sym]->act == act && keymap[sym]->CheckPlayer(player)) {
            return true;
        }
    }
    return false;
}
