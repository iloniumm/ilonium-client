// dear imgui: Platform Backend for SDL 1.2
// Custom backend for Armagetron/RetroyCycles
#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_impl_sdl1.h"

#include <SDL.h>
#include <time.h>

static Uint32 g_Time = 0;
static float g_ScrollDelta = 0.0f;

static ImGuiKey SDL1_KeycodeToImGuiKey(int keycode) {
    switch (keycode) {
        case SDLK_TAB: return ImGuiKey_Tab;
        case SDLK_LEFT: return ImGuiKey_LeftArrow;
        case SDLK_RIGHT: return ImGuiKey_RightArrow;
        case SDLK_UP: return ImGuiKey_UpArrow;
        case SDLK_DOWN: return ImGuiKey_DownArrow;
        case SDLK_PAGEUP: return ImGuiKey_PageUp;
        case SDLK_PAGEDOWN: return ImGuiKey_PageDown;
        case SDLK_HOME: return ImGuiKey_Home;
        case SDLK_END: return ImGuiKey_End;
        case SDLK_INSERT: return ImGuiKey_Insert;
        case SDLK_DELETE: return ImGuiKey_Delete;
        case SDLK_BACKSPACE: return ImGuiKey_Backspace;
        case SDLK_SPACE: return ImGuiKey_Space;
        case SDLK_RETURN: return ImGuiKey_Enter;
        case SDLK_ESCAPE: return ImGuiKey_Escape;
        case SDLK_LCTRL: case SDLK_RCTRL: return ImGuiKey_ModCtrl;
        case SDLK_LSHIFT: case SDLK_RSHIFT: return ImGuiKey_ModShift;
        case SDLK_LALT: case SDLK_RALT: return ImGuiKey_ModAlt;
        case SDLK_a: return ImGuiKey_A;
        case SDLK_c: return ImGuiKey_C;
        case SDLK_v: return ImGuiKey_V;
        case SDLK_x: return ImGuiKey_X;
        case SDLK_y: return ImGuiKey_Y;
        case SDLK_z: return ImGuiKey_Z;
        default: return ImGuiKey_None;
    }
}

bool ImGui_ImplSDL1_Init() {
    ImGuiIO& io = ImGui::GetIO();

    io.BackendPlatformName = "imgui_impl_sdl1";

    // Keyboard mapping - tell ImGui we support these
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Enable SDL Unicode key translation so event->key.keysym.unicode is populated
    SDL_EnableUNICODE(1);

    g_Time = SDL_GetTicks();
    return true;
}

void ImGui_ImplSDL1_Shutdown() {
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = nullptr;
}

bool ImGui_ImplSDL1_ProcessEvent(const SDL_Event* event) {
    ImGuiIO& io = ImGui::GetIO();

    switch (event->type) {
        case SDL_MOUSEMOTION: {
            io.AddMousePosEvent((float)event->motion.x, (float)event->motion.y);
            return true;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            if (event->type == SDL_MOUSEBUTTONDOWN) {
                if (event->button.button == 4) { // SDL_BUTTON_WHEELUP
                    io.AddMouseWheelEvent(0.0f, 1.0f);
                    g_ScrollDelta += 1.0f;
                    return true;
                }
                if (event->button.button == 5) { // SDL_BUTTON_WHEELDOWN
                    io.AddMouseWheelEvent(0.0f, -1.0f);
                    g_ScrollDelta -= 1.0f;
                    return true;
                }
            }
            int button = -1;
            if (event->button.button == SDL_BUTTON_LEFT)   button = 0;
            if (event->button.button == SDL_BUTTON_RIGHT)  button = 1;
            if (event->button.button == SDL_BUTTON_MIDDLE) button = 2;
            if (button >= 0) {
                io.AddMouseButtonEvent(button, (event->type == SDL_MOUSEBUTTONDOWN));
            }
            return true;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            ImGuiKey key = SDL1_KeycodeToImGuiKey(event->key.keysym.sym);
            if (key != ImGuiKey_None) {
                io.AddKeyEvent(key, (event->type == SDL_KEYDOWN));
            }
            // Handle text input for printable characters including Cyrillic / Unicode
            if (event->type == SDL_KEYDOWN) {
                Uint16 c = event->key.keysym.unicode;
                if (c >= 32) {
                    io.AddInputCharacter(c);
                }
            }
            return true;
        }
    }
    return false;
}

void ImGui_ImplSDL1_NewFrame() {
    ImGuiIO& io = ImGui::GetIO();

    io.MouseWheel = g_ScrollDelta;
    g_ScrollDelta = 0.0f;

    // Setup display size
    SDL_Surface* screen = SDL_GetVideoSurface();
    if (screen) {
        io.DisplaySize = ImVec2((float)screen->w, (float)screen->h);
    } else {
        const SDL_VideoInfo* info = SDL_GetVideoInfo();
        if (info) {
            io.DisplaySize = ImVec2((float)info->current_w, (float)info->current_h);
        }
    }
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // Setup time step
    Uint32 current_time = SDL_GetTicks();
    Uint32 elapsed = current_time - g_Time;
    io.DeltaTime = elapsed > 0 ? (float)elapsed / 1000.0f : (1.0f / 60.0f);
    if (io.DeltaTime <= 0.0f) io.DeltaTime = 1.0f / 60.0f;
    g_Time = current_time;

    // Update mouse position (SDL 1.2 doesn't have window focus API like SDL2)
    int mx, my;
    Uint8 buttons = SDL_GetMouseState(&mx, &my);
    io.AddMousePosEvent((float)mx, (float)my);
}

#endif // IMGUI_DISABLE
