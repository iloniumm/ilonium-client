#ifndef VISUALS_MANAGER_H
#define VISUALS_MANAGER_H

#include "tMath.h"

#ifdef LINUX
#include <SDL3/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Struct representing a single physical particle
struct VisualParticle {
    float px, py, pz; // Position
    float vx, vy, vz; // Velocity
    float r, g, b, a; // Color
    float life;       // Remaining lifetime in seconds
    float maxLife;    // Total lifetime
    float size;       // Particle scale
    bool active;
    int type; // 0: death burst/standard, 1: floor dust/ambient
};

// Struct tracking the Screen Shake status
struct VisualScreenShake {
    float duration;
    float intensity;
    float x, y, z;
};

class VisualsManager {
public:
    static void Init();
    static void Shutdown();
    static void Update(float dt);
    
    // Particles management
    static void SpawnDeathBurst(float x, float y, float r, float g, float b);
    static void RenderParticles();
    
    // Camera View Matrix Shaking
    static void TriggerScreenShake(float duration, float intensity);
    static void ApplyCameraShake();

    static bool isInitialized;
    static VisualScreenShake shake;
};

#endif // VISUALS_MANAGER_H
