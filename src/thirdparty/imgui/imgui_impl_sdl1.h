// dear imgui: Platform Backend for SDL 1.2
// Custom backend for Armagetron/RetroyCycles (SDL 1.2 + OpenGL 1.x)
#pragma once
#include "imgui.h"
#ifndef IMGUI_DISABLE

#include <SDL.h>

IMGUI_IMPL_API bool ImGui_ImplSDL1_Init();
IMGUI_IMPL_API void ImGui_ImplSDL1_Shutdown();
IMGUI_IMPL_API void ImGui_ImplSDL1_NewFrame();
IMGUI_IMPL_API bool ImGui_ImplSDL1_ProcessEvent(const SDL_Event* event);

#endif
