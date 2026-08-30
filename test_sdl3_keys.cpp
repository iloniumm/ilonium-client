#include <SDL3/SDL.h>
#include <iostream>

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "SDL3 Key/Scancode test:\n";

    // Test a few common letters
    SDL_Scancode scancodes[] = {
        SDL_SCANCODE_A, SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_D,
        SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
        SDL_SCANCODE_ESCAPE, SDL_SCANCODE_SPACE, SDL_SCANCODE_RETURN
    };

    for (auto sc : scancodes) {
        SDL_Keycode key = SDL_GetKeyFromScancode(sc, SDL_KMOD_NONE, false);
        const char* scName = SDL_GetScancodeName(sc);
        const char* keyName = SDL_GetKeyName(key);
        std::cout << "Scancode: " << sc << " (" << (scName ? scName : "NULL") << ") -> Keycode: " << key 
                  << " (" << (keyName ? keyName : "NULL") << ")\n";
    }

    // Also let's print the key name of direct legacy values like 'w' = 119, etc.
    std::cout << "Legacy key name tests:\n";
    std::cout << "SDL_GetKeyName(119): " << SDL_GetKeyName(static_cast<SDL_Keycode>(119)) << "\n";
    std::cout << "SDL_GetKeyName(97): " << SDL_GetKeyName(static_cast<SDL_Keycode>(97)) << "\n";
    std::cout << "SDL_GetKeyName(273): " << SDL_GetKeyName(static_cast<SDL_Keycode>(273)) << "\n"; // In SDL3, SDLK_UP is 1073741906, what is SDL_GetKeyName(273)?

    SDL_Quit();
    return 0;
}
