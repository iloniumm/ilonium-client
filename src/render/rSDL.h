#ifndef AT_SDL_H
#define AT_SDL_H

#include "config.h"

#ifndef DEDICATED
#include <SDL3/SDL.h>
#define SDL_OPENGL 1
#define SDLK_LAST 512

#define SDL_GRAB_ON true
#define SDL_GRAB_OFF false

inline void SDL_WM_GrabInput(bool grab) {
    extern SDL_Window *sr_screen;
    SDL_SetWindowMouseGrab(sr_screen, grab);
}

inline SDL_MouseButtonFlags SDL_GetRelativeMouseState(int *x, int *y) {
    float fx = 0, fy = 0;
    SDL_MouseButtonFlags mask = SDL_GetRelativeMouseState(&fx, &fy);
    if (x) *x = (int)fx;
    if (y) *y = (int)fy;
    return mask;
}

inline SDL_MouseButtonFlags SDL_GetMouseState(int *x, int *y) {
    float fx = 0, fy = 0;
    SDL_MouseButtonFlags mask = SDL_GetMouseState(&fx, &fy);
    if (x) *x = (int)fx;
    if (y) *y = (int)fy;
    return mask;
}

inline SDL_Scancode SDL_GetScancodeFromKey(SDL_Keycode key) {
    return ::SDL_GetScancodeFromKey(key, NULL);
}

inline char SDL_TranslateCyrillic(unsigned char b1, unsigned char b2) {
    if (b1 == 209 && b2 == 145) return '`';
    if (b1 == 208 && b2 == 129) return '~';
    if (b1 == 208 && b2 == 185) return 'q';
    if (b1 == 208 && b2 == 153) return 'Q';
    if (b1 == 209 && b2 == 134) return 'w';
    if (b1 == 208 && b2 == 166) return 'W';
    if (b1 == 209 && b2 == 131) return 'e';
    if (b1 == 208 && b2 == 163) return 'E';
    if (b1 == 208 && b2 == 186) return 'r';
    if (b1 == 208 && b2 == 154) return 'R';
    if (b1 == 208 && b2 == 181) return 't';
    if (b1 == 208 && b2 == 149) return 'T';
    if (b1 == 208 && b2 == 189) return 'y';
    if (b1 == 208 && b2 == 157) return 'Y';
    if (b1 == 208 && b2 == 179) return 'u';
    if (b1 == 208 && b2 == 147) return 'U';
    if (b1 == 209 && b2 == 136) return 'i';
    if (b1 == 208 && b2 == 168) return 'I';
    if (b1 == 209 && b2 == 137) return 'o';
    if (b1 == 208 && b2 == 169) return 'O';
    if (b1 == 208 && b2 == 183) return 'p';
    if (b1 == 208 && b2 == 151) return 'P';
    if (b1 == 209 && b2 == 133) return '[';
    if (b1 == 208 && b2 == 165) return '{';
    if (b1 == 209 && b2 == 138) return ']';
    if (b1 == 208 && b2 == 170) return '}';
    if (b1 == 209 && b2 == 132) return 'a';
    if (b1 == 208 && b2 == 164) return 'A';
    if (b1 == 209 && b2 == 139) return 's';
    if (b1 == 208 && b2 == 171) return 'S';
    if (b1 == 208 && b2 == 178) return 'd';
    if (b1 == 208 && b2 == 146) return 'D';
    if (b1 == 208 && b2 == 176) return 'f';
    if (b1 == 208 && b2 == 144) return 'F';
    if (b1 == 208 && b2 == 191) return 'g';
    if (b1 == 208 && b2 == 159) return 'G';
    if (b1 == 209 && b2 == 128) return 'h';
    if (b1 == 208 && b2 == 160) return 'H';
    if (b1 == 208 && b2 == 190) return 'j';
    if (b1 == 208 && b2 == 158) return 'J';
    if (b1 == 208 && b2 == 187) return 'k';
    if (b1 == 208 && b2 == 155) return 'K';
    if (b1 == 208 && b2 == 180) return 'l';
    if (b1 == 208 && b2 == 148) return 'L';
    if (b1 == 208 && b2 == 182) return ';';
    if (b1 == 208 && b2 == 150) return ':';
    if (b1 == 209 && b2 == 141) return '\'';
    if (b1 == 208 && b2 == 173) return '"';
    if (b1 == 209 && b2 == 143) return 'z';
    if (b1 == 208 && b2 == 175) return 'Z';
    if (b1 == 209 && b2 == 135) return 'x';
    if (b1 == 208 && b2 == 167) return 'X';
    if (b1 == 209 && b2 == 129) return 'c';
    if (b1 == 208 && b2 == 161) return 'C';
    if (b1 == 208 && b2 == 188) return 'v';
    if (b1 == 208 && b2 == 156) return 'V';
    if (b1 == 208 && b2 == 184) return 'b';
    if (b1 == 208 && b2 == 152) return 'B';
    if (b1 == 209 && b2 == 130) return 'n';
    if (b1 == 208 && b2 == 162) return 'N';
    if (b1 == 209 && b2 == 140) return 'm';
    if (b1 == 208 && b2 == 172) return 'M';
    if (b1 == 208 && b2 == 177) return ',';
    if (b1 == 208 && b2 == 145) return '<';
    if (b1 == 209 && b2 == 142) return '.';
    if (b1 == 208 && b2 == 174) return '>';
    return 0;
}

#define SDL_BUTTON(X) SDL_BUTTON_MASK(X)
#define KMOD_SHIFT SDL_KMOD_SHIFT
#define KMOD_CTRL SDL_KMOD_CTRL
#define KMOD_ALT SDL_KMOD_ALT
#define KMOD_GUI SDL_KMOD_GUI
#else
#define SDLK_LAST 1024
typedef int SDL_keysym;
typedef int SDL_Event;
typedef unsigned char Uint8;
typedef unsigned int Uint32;
typedef int SDL_AudioSpec;
#endif

#endif
