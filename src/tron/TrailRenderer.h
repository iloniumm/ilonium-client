#ifndef TRAIL_RENDERER_H
#define TRAIL_RENDERER_H

#include <vector>
#include <SDL3/SDL.h>


// Simple vector structure
struct vec4 {
    float x, y, z, w;
};

// Simple 4x4 matrix structure
struct mat4 {
    float m[16];
    
    static mat4 identity() {
        mat4 mat = {0};
        mat.m[0] = 1.0f; mat.m[5] = 1.0f; mat.m[10] = 1.0f; mat.m[15] = 1.0f;
        return mat;
    }
    
    static mat4 transformation(float x1, float y1, float x2, float y2, float height, float extrarise = 0.0f) {
        mat4 mat = {0};
        float dx = x2 - x1;
        float dy = y2 - y1;
        
        // Column 0: X axis (direction and length)
        mat.m[0] = dx;
        mat.m[1] = dy;
        mat.m[2] = 0.0f;
        mat.m[3] = 0.0f;
        
        // Column 1: Y axis (perpendicular vector to prevent degenerate matrices)
        mat.m[4] = -dy;
        mat.m[5] = dx;
        mat.m[6] = 0.0f;
        mat.m[7] = 0.0f;
        
        // Column 2: Z axis (height)
        mat.m[8] = 0.0f;
        mat.m[9] = 0.0f;
        mat.m[10] = height;
        mat.m[11] = 0.0f;
        
        // Column 3: Translation
        mat.m[12] = x1;
        mat.m[13] = y1;
        mat.m[14] = extrarise;
        mat.m[15] = 1.0f;
        
        return mat;
    }
};

// Trail segment holding model transformation, color and texture coordinates
struct TrailSegment {
    mat4 transform;
    vec4 color;
    vec4 texCoords;
};

// Instance Data structure uploaded to the GPU
struct InstanceData {
    mat4 model;
    float color[4];
    float texCoords[4];
};

class TrailRenderer {
public:
    TrailRenderer();
    ~TrailRenderer();
    
    void Init();
    void RenderTrails(const std::vector<TrailSegment>& segments, const float* viewMatrix, const float* projectionMatrix);
    void RenderFallback(const std::vector<TrailSegment>& segments);
    void Cleanup();
    
private:
    unsigned int vaoID;
    unsigned int quadVboID;
    unsigned int instanceVboID;
    unsigned int shaderProgram;
    int uViewLoc;
    int uProjLoc;
    
    void CompileShaders();
};

extern std::vector<TrailSegment> g_collectedTrails;
extern TrailRenderer g_trailRenderer;
extern bool cfg_EnableInstancing;

#endif // TRAIL_RENDERER_H
