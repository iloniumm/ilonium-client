#include "VisualsManager.h"
#include "gMenus.h"
#include "gWall.h"
#include "gCycle.h"
#include "gWinZone.h"
#include "eGrid.h"
#include "eRectangle.h"
#include "../engine/ePlayer.h"
#include "TrailRenderer.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

// Static definitions
bool VisualsManager::isInitialized = false;
VisualScreenShake VisualsManager::shake = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

// Particle Pool
#define MAX_PARTICLES 4096
static VisualParticle g_particles[MAX_PARTICLES];
static int g_activeParticles[MAX_PARTICLES];
static int g_activeCount = 0;

// Last camera position tracked dynamically
static float g_lastCamX = 0.0f;
static float g_lastCamY = 0.0f;

// Death warp parameters
static float g_deathPosX = 0.0f;
static float g_deathPosY = 0.0f;
static float g_timeSinceDeath = -1.0f;


void VisualsManager::Init() {
    if (isInitialized) return;
    
    // Clear particle list
    for (int i = 0; i < MAX_PARTICLES; i++) {
        g_particles[i].active = false;
        g_particles[i].type = 0;
    }
    g_activeCount = 0;

    // Initialize trail renderer (loads shaders & buffers)
    g_trailRenderer.Init();

    isInitialized = true;
    std::cout << "[VisualsManager] Initialized particle & shake systems." << std::endl;
}

void VisualsManager::Shutdown() {
    if (!isInitialized) return;
    
    g_trailRenderer.Cleanup();
    
    isInitialized = false;
}

// Spawns a single particle (optimized O(1) circular buffer allocation)
static void SpawnParticle(float px, float py, float pz, float vx, float vy, float vz, float r, float g, float b, float a, float life, float size, int type = 0) {
    static int nextSlot = 0;
    VisualParticle& p = g_particles[nextSlot];
    bool wasActive = p.active;
    p.px = px; p.py = py; p.pz = pz;
    p.vx = vx; p.vy = vy; p.vz = vz;
    p.r = r; p.g = g; p.b = b; p.a = a;
    p.life = life; p.maxLife = life;
    p.size = size;
    p.active = true;
    p.type = type;
    if (!wasActive) {
        if (g_activeCount < MAX_PARTICLES) {
            g_activeParticles[g_activeCount++] = nextSlot;
        }
    }
    nextSlot = (nextSlot + 1) % MAX_PARTICLES;
}

void VisualsManager::SpawnDeathBurst(float x, float y, float r, float g, float b) {
    // Record Death Event details for Vertex Shader Black Hole
    g_deathPosX = x;
    g_deathPosY = y;
    g_timeSinceDeath = 0.0f;

    extern bool sg_modParticleSystemEnabled;
    if (!sg_modParticleSystemEnabled) return;

    extern int sg_modDeathParticlesCount;
    int count = sg_modDeathParticlesCount;
    if (count <= 0) return;

    for (int i = 0; i < count; i++) {
        float phi = (rand() % 360) * 3.14159f / 180.0f;
        float theta = (rand() % 180) * 3.14159f / 180.0f;
        float speed = 2.0f + (rand() % 1200) / 100.0f; // 2.0 to 14.0 speed

        float vx = sin(theta) * cos(phi) * speed;
        float vy = sin(theta) * sin(phi) * speed;
        float vz = cos(theta) * speed + 2.0f; // upward bias

        float life = 0.8f + (rand() % 120) / 100.0f; // 0.8 to 2.0 seconds
        float size = 0.08f + (rand() % 10) / 100.0f;

        SpawnParticle(x, y, 0.5f, vx, vy, vz, r, g, b, 1.0f, life, size);
    }
}

void VisualsManager::TriggerScreenShake(float duration, float intensity) {
    shake.duration = duration;
    shake.intensity = intensity;
}

void VisualsManager::ApplyCameraShake() {
    extern bool sg_modScreenShakeEnabled;
    if (sg_modScreenShakeEnabled && shake.duration > 0.0f) {
        glTranslatef(shake.x, shake.y, shake.z);
    }
}

void VisualsManager::Update(float dt) {
    if (dt <= 0.0f) return;

    // Update screen shake
    if (shake.duration > 0.0f) {
        shake.duration -= dt;
        if (shake.duration < 0.0f) shake.duration = 0.0f;
        float factor = shake.duration;
        shake.x = (((rand() % 2000) - 1000) / 1000.0f) * shake.intensity * factor;
        shake.y = (((rand() % 2000) - 1000) / 1000.0f) * shake.intensity * factor;
        shake.z = (((rand() % 2000) - 1000) / 1000.0f) * shake.intensity * factor;
    } else {
        shake.x = shake.y = shake.z = 0.0f;
    }

    // Update vertex shader death warp time
    if (g_timeSinceDeath >= 0.0f) {
        g_timeSinceDeath += dt;
        if (g_timeSinceDeath > 3.0f) {
            g_timeSinceDeath = -1.0f;
        }
    }

    // Update particles physics
    extern bool sg_modParticleSystemEnabled;
    if (sg_modParticleSystemEnabled) {
        float friction = 0.96f; // air friction
        float frictionFactor = pow(friction, dt * 60.0f); // O(1) pow optimization
        int writeIdx = 0;
        for (int i = 0; i < g_activeCount; i++) {
            int idx = g_activeParticles[i];
            VisualParticle& p = g_particles[idx];
            if (p.active) {
                p.life -= dt;
                if (p.life <= 0.0f) {
                    p.active = false;
                    continue;
                }

                p.px += p.vx * dt;
                p.py += p.vy * dt;
                p.pz += p.vz * dt;

                p.vx *= frictionFactor;
                p.vy *= frictionFactor;
                p.vz *= frictionFactor;

                p.a = p.life / p.maxLife; // Fade out
                g_activeParticles[writeIdx++] = idx;
            }
        }
        g_activeCount = writeIdx;
    }

    // Ambient Grid Dust Particles
    extern bool sg_modAmbientParticlesEnabled;
    extern int sg_modAmbientParticlesMode;
    extern int sg_modAmbientParticlesMin;
    extern int sg_modAmbientParticlesMax;

    if (sg_modAmbientParticlesEnabled) {
        // Find if there is any active gZone in the current grid
        gZone* activeZone = NULL;
        eGrid* grid = eGrid::CurrentGrid();
        if (grid) {
            const tList<eGameObject>& gameObjects = grid->GameObjects();
            for (int j = 0; j < gameObjects.Len(); ++j) {
                gZone* z = dynamic_cast<gZone*>(gameObjects(j));
                if (z) {
                    activeZone = z;
                    break;
                }
            }
        }

        // Count active ambient particles (type == 1)
        int ambientCount = 0;
        for (int i = 0; i < g_activeCount; i++) {
            int idx = g_activeParticles[i];
            if (g_particles[idx].active && g_particles[idx].type == 1) {
                ambientCount++;
            }
        }

        // Determine how many we want to spawn
        int toSpawn = 0;
        if (ambientCount < sg_modAmbientParticlesMin) {
            toSpawn = sg_modAmbientParticlesMin - ambientCount;
        } else if (ambientCount < sg_modAmbientParticlesMax) {
            toSpawn = 2; // Spawn 2 per frame to reach max smoothly
        }

        // Limit spawning per frame to avoid performance spikes
        if (toSpawn > 20) toSpawn = 20;

        // Get map boundaries
        eRectangle bounds = eWallRim::GetBounds();
        eCoord low = bounds.GetLow();
        eCoord high = bounds.GetHigh();
        float mapWidth = high.x - low.x;
        float mapHeight = high.y - low.y;

        for (int k = 0; k < toSpawn; k++) {
            float px = 0.0f;
            float py = 0.0f;

            if (sg_modAmbientParticlesMode == 1) { // Zone Only
                if (activeZone) {
                    float angle = (rand() % 360) * 3.14159f / 180.0f;
                    float r_dist = (rand() % 1000) / 1000.0f * activeZone->GetRadius();
                    px = activeZone->GetPosition().x + cos(angle) * r_dist;
                    py = activeZone->GetPosition().y + sin(angle) * r_dist;
                } else {
                    // No active zone in zone only mode: don't spawn anything
                    continue;
                }
            } else { // Uniform (Mode 0)
                if (mapWidth > 1.0f && mapHeight > 1.0f) {
                    px = low.x + (rand() % 1000) / 1000.0f * mapWidth;
                    py = low.y + (rand() % 1000) / 1000.0f * mapHeight;
                } else {
                    px = 0.0f;
                    py = 0.0f;
                }
            }

            float pz = 0.0f;

            float vx = ((rand() % 100) - 50) / 100.0f * 0.2f;
            float vy = ((rand() % 100) - 50) / 100.0f * 0.2f;
            float vz = 0.5f + (rand() % 150) / 100.0f; // vertical ascent

            // Glowing green/blue digital dust color
            float r = 0.0f;
            float g = 0.8f + (rand() % 20) / 100.0f;
            float b = 0.7f + (rand() % 30) / 100.0f;

            float life = 2.0f + (rand() % 200) / 100.0f; // 2-4 seconds life
            float size = 0.04f + (rand() % 4) / 100.0f;

            SpawnParticle(px, py, pz, vx, vy, vz, r, g, b, 0.8f, life, size, 1);
        }
    }
}

void VisualsManager::RenderParticles() {
    extern bool sg_modParticleSystemEnabled;
    if (!sg_modParticleSystemEnabled) return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive glow
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glDisable(GL_TEXTURE_2D);

    // Extract billboard right/up vectors
    GLfloat mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    float rx = mv[0];
    float ry = mv[4];
    float rz = mv[8];
    float ux = mv[1];
    float uy = mv[5];
    float uz = mv[9];

    // Capture camera translation for ambient spawning (corrected column-major matrix mapping)
    g_lastCamX = -(mv[0]*mv[12] + mv[1]*mv[13] + mv[2]*mv[14]);
    g_lastCamY = -(mv[4]*mv[12] + mv[5]*mv[13] + mv[6]*mv[14]);

    glBegin(GL_QUADS);
    for (int i = 0; i < g_activeCount; i++) {
        int idx = g_activeParticles[i];
        const VisualParticle& p = g_particles[idx];
        glColor4f(p.r, p.g, p.b, p.a);
        
        float s = p.size;
        glVertex3f(p.px - rx*s - ux*s, p.py - ry*s - uy*s, p.pz - rz*s - uz*s);
        glVertex3f(p.px + rx*s - ux*s, p.py + ry*s - uy*s, p.pz + rz*s - uz*s);
        glVertex3f(p.px + rx*s + ux*s, p.py + ry*s + uy*s, p.pz + rz*s + uz*s);
        glVertex3f(p.px - rx*s + ux*s, p.py - ry*s + uy*s, p.pz - rz*s - uz*s);
    }
    glEnd();

    glPopAttrib();
}
