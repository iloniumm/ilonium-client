#include "TrailRenderer.h"
#include <iostream>

// Global variables
bool cfg_EnableInstancing = true;
std::vector<TrailSegment> g_collectedTrails;
TrailRenderer g_trailRenderer;

// GL Function Pointers
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays_ptr = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray_ptr = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays_ptr = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers_ptr = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer_ptr = nullptr;
PFNGLBUFFERDATAPROC glBufferData_ptr = nullptr;
PFNGLDELETEBUFFERSPROC glDeleteBuffers_ptr = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ptr = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray_ptr = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_ptr = nullptr;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor_ptr = nullptr;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced_ptr = nullptr;
PFNGLCREATESHADERPROC glCreateShader_ptr = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource_ptr = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader_ptr = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv_ptr = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_ptr = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram_ptr = nullptr;
PFNGLATTACHSHADERPROC glAttachShader_ptr = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram_ptr = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv_ptr = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_ptr = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram_ptr = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram_ptr = nullptr;
PFNGLDELETESHADERPROC glDeleteShader_ptr = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ptr = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_ptr = nullptr;
PFNGLUNIFORM1IPROC glUniform1i_ptr = nullptr;

#define glGenVertexArrays glGenVertexArrays_ptr
#define glBindVertexArray glBindVertexArray_ptr
#define glDeleteVertexArrays glDeleteVertexArrays_ptr
#define glGenBuffers glGenBuffers_ptr
#define glBindBuffer glBindBuffer_ptr
#define glBufferData glBufferData_ptr
#define glDeleteBuffers glDeleteBuffers_ptr
#define glEnableVertexAttribArray glEnableVertexAttribArray_ptr
#define glDisableVertexAttribArray glDisableVertexAttribArray_ptr
#define glVertexAttribPointer glVertexAttribPointer_ptr
#define glVertexAttribDivisor glVertexAttribDivisor_ptr
#define glDrawArraysInstanced glDrawArraysInstanced_ptr
#define glCreateShader glCreateShader_ptr
#define glShaderSource glShaderSource_ptr
#define glCompileShader glCompileShader_ptr
#define glGetShaderiv glGetShaderiv_ptr
#define glGetShaderInfoLog glGetShaderInfoLog_ptr
#define glCreateProgram glCreateProgram_ptr
#define glAttachShader glAttachShader_ptr
#define glLinkProgram glLinkProgram_ptr
#define glGetProgramiv glGetProgramiv_ptr
#define glGetProgramInfoLog glGetProgramInfoLog_ptr
#define glUseProgram glUseProgram_ptr
#define glDeleteProgram glDeleteProgram_ptr
#define glDeleteShader glDeleteShader_ptr
#define glGetUniformLocation glGetUniformLocation_ptr
#define glUniformMatrix4fv glUniformMatrix4fv_ptr
#define glUniform1i glUniform1i_ptr

static void LoadGLExtensions() {
    static bool loaded = false;
    if (loaded) return;
    glGenVertexArrays_ptr = (PFNGLGENVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glGenVertexArrays");
    glBindVertexArray_ptr = (PFNGLBINDVERTEXARRAYPROC)SDL_GL_GetProcAddress("glBindVertexArray");
    glDeleteVertexArrays_ptr = (PFNGLDELETEVERTEXARRAYSPROC)SDL_GL_GetProcAddress("glDeleteVertexArrays");
    glGenBuffers_ptr = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
    glBindBuffer_ptr = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
    glBufferData_ptr = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
    glDeleteBuffers_ptr = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
    glEnableVertexAttribArray_ptr = (PFNGLENABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    glDisableVertexAttribArray_ptr = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glDisableVertexAttribArray");
    glVertexAttribPointer_ptr = (PFNGLVERTEXATTRIBPOINTERPROC)SDL_GL_GetProcAddress("glVertexAttribPointer");
    glVertexAttribDivisor_ptr = (PFNGLVERTEXATTRIBDIVISORPROC)SDL_GL_GetProcAddress("glVertexAttribDivisor");
    glDrawArraysInstanced_ptr = (PFNGLDRAWARRAYSINSTANCEDPROC)SDL_GL_GetProcAddress("glDrawArraysInstanced");
    glCreateShader_ptr = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
    glShaderSource_ptr = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
    glCompileShader_ptr = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
    glGetShaderiv_ptr = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
    glGetShaderInfoLog_ptr = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glCreateProgram_ptr = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
    glAttachShader_ptr = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
    glLinkProgram_ptr = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
    glGetProgramiv_ptr = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
    glGetProgramInfoLog_ptr = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
    glUseProgram_ptr = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
    glDeleteProgram_ptr = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
    glDeleteShader_ptr = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
    glGetUniformLocation_ptr = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
    glUniformMatrix4fv_ptr = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");
    glUniform1i_ptr = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
    loaded = true;
}

TrailRenderer::TrailRenderer() 
    : vaoID(0), quadVboID(0), instanceVboID(0), shaderProgram(0), uViewLoc(-1), uProjLoc(-1) {}

TrailRenderer::~TrailRenderer() {
    Cleanup();
}

void TrailRenderer::Init() {
    LoadGLExtensions();
    
    // Generate VBO Buffers (No VAO)
    glGenBuffers(1, &quadVboID);
    glGenBuffers(1, &instanceVboID);
    
    // Setup Static local Quad Geometry
    // 1 Quad = 4 vertices. local layout: x [0..1], y [0], z [0..1]
    static const float quadVertices[] = {
        0.0f, 0.0f, 0.0f, // Bottom-Left
        1.0f, 0.0f, 0.0f, // Bottom-Right
        1.0f, 0.0f, 1.0f, // Top-Right
        0.0f, 0.0f, 1.0f  // Top-Left
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, quadVboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    CompileShaders();
}

void TrailRenderer::Cleanup() {
    if (quadVboID) {
        glDeleteBuffers(1, &quadVboID);
        quadVboID = 0;
    }
    if (instanceVboID) {
        glDeleteBuffers(1, &instanceVboID);
        instanceVboID = 0;
    }
    if (vaoID) {
        glDeleteVertexArrays(1, &vaoID);
        vaoID = 0;
    }
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
}

void TrailRenderer::RenderTrails(const std::vector<TrailSegment>& segments, const float* viewMatrix, const float* projectionMatrix) {
    if (segments.empty()) return;
    
    if (cfg_EnableInstancing) {
        // Gather instance data
        std::vector<InstanceData> instanceBatch;
        instanceBatch.reserve(segments.size());
        
        for (const auto& seg : segments) {
            InstanceData data;
            data.model = seg.transform;
            data.color[0] = seg.color.x;
            data.color[1] = seg.color.y;
            data.color[2] = seg.color.z;
            data.color[3] = seg.color.w;
            data.texCoords[0] = seg.texCoords.x;
            data.texCoords[1] = seg.texCoords.y;
            data.texCoords[2] = seg.texCoords.z;
            data.texCoords[3] = seg.texCoords.w;
            
            instanceBatch.push_back(data);
        }
        
        // Upload data to GPU
        glBindBuffer(GL_ARRAY_BUFFER, instanceVboID);
        glBufferData(GL_ARRAY_BUFFER, instanceBatch.size() * sizeof(InstanceData), instanceBatch.data(), GL_DYNAMIC_DRAW);
        
        // Bind program and uniforms
        glUseProgram(shaderProgram);
        if (uViewLoc != -1) {
            glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, viewMatrix);
        }
        if (uProjLoc != -1) {
            glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, projectionMatrix);
        }
        
        GLint useTextureVal = glIsEnabled(GL_TEXTURE_2D);
        GLint uUseTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");
        if (uUseTextureLoc != -1) {
            glUniform1i(uUseTextureLoc, useTextureVal);
        }
        GLint uWallTextureLoc = glGetUniformLocation(shaderProgram, "wallTexture");
        if (uWallTextureLoc != -1) {
            glUniform1i(uWallTextureLoc, 0);
        }
        
        // Bind and configure Quad VBO attributes (Location 0)
        glBindBuffer(GL_ARRAY_BUFFER, quadVboID);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glVertexAttribDivisor(0, 0); // Per-vertex
        
        // Bind and configure Instance VBO attributes (Locations 1-6)
        glBindBuffer(GL_ARRAY_BUFFER, instanceVboID);
        GLsizei stride = sizeof(InstanceData);
        for (unsigned int i = 0; i < 4; ++i) {
            unsigned int attribLoc = 1 + i;
            glEnableVertexAttribArray(attribLoc);
            glVertexAttribPointer(attribLoc, 4, GL_FLOAT, GL_FALSE, stride, (void*)(i * 4 * sizeof(float)));
            glVertexAttribDivisor(attribLoc, 1); // Per-instance
        }
        
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride, (void*)(16 * sizeof(float)));
        glVertexAttribDivisor(5, 1); // Per-instance

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride, (void*)(20 * sizeof(float)));
        glVertexAttribDivisor(6, 1); // Per-instance
        
        // Save current blend and face culling states to prevent state bleed-through
        GLboolean cullEnabled = glIsEnabled(GL_CULL_FACE);
        GLboolean blendEnabled = glIsEnabled(GL_BLEND);
        GLint blendSrc, blendDst;
        glGetIntegerv(GL_BLEND_SRC, &blendSrc);
        glGetIntegerv(GL_BLEND_DST, &blendDst);
        
        // Apply state overrides for instanced trails rendering
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Draw batch
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, static_cast<GLsizei>(instanceBatch.size()));
        
        // Restore saved blend and face culling states
        if (cullEnabled) {
            glEnable(GL_CULL_FACE);
        }
        if (!blendEnabled) {
            glDisable(GL_BLEND);
        } else {
            glBlendFunc(blendSrc, blendDst);
        }
        
        // Cleanup attributes and reset divisors to prevent bleed-through into legacy code!
        glDisableVertexAttribArray(0);
        glVertexAttribDivisor(0, 0);
        for (unsigned int i = 0; i < 4; ++i) {
            glDisableVertexAttribArray(1 + i);
            glVertexAttribDivisor(1 + i, 0);
        }
        glDisableVertexAttribArray(5);
        glVertexAttribDivisor(5, 0);
        glDisableVertexAttribArray(6);
        glVertexAttribDivisor(6, 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
    } else {
        // Safe Fallback path
        RenderFallback(segments);
    }
}

void TrailRenderer::RenderFallback(const std::vector<TrailSegment>& segments) {
    glBegin(GL_QUADS);
    for (const auto& seg : segments) {
        // Quad vertices extracted from transformation matrix translation/scale
        // Column 3 is translation (m[12], m[13], m[14])
        // Column 0 is scale/X-axis vector
        // Column 2 is scale/Z-axis height vector
        float x1 = seg.transform.m[12];
        float y1 = seg.transform.m[13];
        
        float dx = seg.transform.m[0];
        float dy = seg.transform.m[1];
        
        float x2 = x1 + dx;
        float y2 = y1 + dy;
        
        float height = seg.transform.m[10];
        
        float ta = seg.texCoords.x;
        float te = seg.texCoords.y;
        float hfrac = seg.texCoords.z;
        float a_top = seg.texCoords.w;
        float a_bottom = seg.color.w;
        
        // Bottom-Left
        glColor4f(seg.color.x, seg.color.y, seg.color.z, a_bottom);
        glTexCoord2f(ta, hfrac);
        glVertex3f(x1, y1, 0.0f);
        
        // Bottom-Right
        glColor4f(seg.color.x, seg.color.y, seg.color.z, a_bottom);
        glTexCoord2f(te, hfrac);
        glVertex3f(x2, y2, 0.0f);
        
        // Top-Right
        glColor4f(seg.color.x, seg.color.y, seg.color.z, a_top);
        glTexCoord2f(te, 0.0f);
        glVertex3f(x2, y2, height);
        
        // Top-Left
        glColor4f(seg.color.x, seg.color.y, seg.color.z, a_top);
        glTexCoord2f(ta, 0.0f);
        glVertex3f(x1, y1, height);
    }
    glEnd();
}

void TrailRenderer::CompileShaders() {
    const char* vertexSource = 
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "layout(location = 1) in mat4 aInstanceMatrix;\n"
        "layout(location = 5) in vec4 aInstanceColor;\n"
        "layout(location = 6) in vec4 aInstanceTexCoords;\n" // x=ta, y=te, z=hfrac, w=a_top
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec4 fragColor;\n"
        "out vec2 fragTexCoord;\n"
        "out vec3 localPos;\n"
        "void main() {\n"
        "    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);\n"
        "    \n"
        "    // Interpolate alpha from bottom (aPos.z = 0) to top (aPos.z = 1)\n"
        "    float alpha = mix(aInstanceColor.a, aInstanceTexCoords.w, aPos.z);\n"
        "    fragColor = vec4(aInstanceColor.rgb, alpha);\n"
        "    localPos = aPos;\n"
        "    \n"
        "    // Interpolate texture coordinates based on local position x (0..1) and z (0..1)\n"
        "    float u = mix(aInstanceTexCoords.x, aInstanceTexCoords.y, aPos.x);\n"
        "    float v = mix(aInstanceTexCoords.z, 0.0, aPos.z);\n" // vertical texture coordinate: bottom is hfrac, top is 0.0
        "    fragTexCoord = vec2(u, v);\n"
        "}\n";

    const char* fragmentSource = 
        "#version 330 core\n"
        "uniform sampler2D wallTexture;\n"
        "uniform bool useTexture;\n"
        "in vec4 fragColor;\n"
        "in vec2 fragTexCoord;\n"
        "in vec3 localPos;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "    vec4 texCol = useTexture ? texture(wallTexture, fragTexCoord) : vec4(1.0);\n"
        "    vec4 finalColor = fragColor * texCol;\n"
        "    \n"
        "    // Ambient height-based alpha gradient fade at the base of the trail wall\n"
        "    float heightFade = smoothstep(0.0, 0.2, localPos.z);\n"
        "    finalColor.a *= (0.3 + 0.7 * heightFade);\n"
        "    FragColor = clamp(finalColor, 0.0, 1.0);\n"
        "}\n";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexSource, nullptr);
    glCompileShader(vs);

    GLint success;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vs, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader compilation failed:\n" << infoLog << std::endl;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentSource, nullptr);
    glCompileShader(fs);
    
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fs, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader compilation failed:\n" << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    uViewLoc = glGetUniformLocation(shaderProgram, "view");
    uProjLoc = glGetUniformLocation(shaderProgram, "projection");
}
