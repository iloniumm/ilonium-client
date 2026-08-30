#include "cPostProcessor.h"
#include "tConfiguration.h"
#include "rScreen.h"
#include "tSysTime.h"
#include <iostream>
#include <fstream>
#include <sstream>

#ifndef DEDICATED

bool sg_postProcessingEnabled = true;
static tConfItem<bool> sg_postProcessingEnabledConf("POST_PROCESSING_ENABLED", sg_postProcessingEnabled);

float sg_vignetteIntensity = 0.0f;
static tConfItem<float> sg_vignetteIntensityConf("VIGNETTE_INTENSITY", sg_vignetteIntensity);

float sg_vignetteRadius = 0.80f;
static tConfItem<float> sg_vignetteRadiusConf("VIGNETTE_RADIUS", sg_vignetteRadius);

float sg_vignetteSoftness = 0.45f;
static tConfItem<float> sg_vignetteSoftnessConf("VIGNETTE_SOFTNESS", sg_vignetteSoftness);

float sg_bloomThreshold = 0.55f;
static tConfItem<float> sg_bloomThresholdConf("BLOOM_THRESHOLD", sg_bloomThreshold);

float sg_bloomStrength = 2.00f;
static tConfItem<float> sg_bloomStrengthConf("BLOOM_STRENGTH", sg_bloomStrength);

float sg_bloomSpread = 0.75f;
static tConfItem<float> sg_bloomSpreadConf("BLOOM_SPREAD", sg_bloomSpread);

bool g_EnhancedGraphicsMode = true;
static tConfItem<bool> conf_enhancedGraphicsMode("ENHANCED_GRAPHICS_MODE", g_EnhancedGraphicsMode);

// OpenGL Function Pointers
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSPROC_CUSTOM) (GLsizei n, GLuint *framebuffers);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFERPROC_CUSTOM) (GLenum target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC_CUSTOM) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSPROC_CUSTOM) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFERPROC_CUSTOM) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEPROC_CUSTOM) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC_CUSTOM) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERPROC_CUSTOM) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC_CUSTOM) (GLenum target);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSPROC_CUSTOM) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSPROC_CUSTOM) (GLsizei n, const GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLTEXIMAGE2DMULTISAMPLEPROC_CUSTOM) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (APIENTRYP PFNGLBLITFRAMEBUFFERPROC_CUSTOM) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC_CUSTOM) (GLenum type);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC_CUSTOM) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC_CUSTOM) (GLuint shader);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC_CUSTOM) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC_CUSTOM) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC_CUSTOM) (void);
typedef void (APIENTRYP PFNGLATTACHSHADERPROC_CUSTOM) (GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC_CUSTOM) (GLuint program);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC_CUSTOM) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC_CUSTOM) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC_CUSTOM) (GLuint program);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPROC_CUSTOM) (GLuint program);
typedef void (APIENTRYP PFNGLDELETESHADERPROC_CUSTOM) (GLuint shader);
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC_CUSTOM) (GLuint program, const GLchar *name);
typedef void (APIENTRYP PFNGLUNIFORM1IPROC_CUSTOM) (GLint location, GLint v0);
typedef void (APIENTRYP PFNGLUNIFORM1FPROC_CUSTOM) (GLint location, GLfloat v0);
typedef void (APIENTRYP PFNGLUNIFORM2FPROC_CUSTOM) (GLint location, GLfloat v0, GLfloat v1);
typedef void (APIENTRYP PFNGLUNIFORM3FPROC_CUSTOM) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC_CUSTOM) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC_CUSTOM) (GLenum texture);
typedef void (APIENTRYP PFNGLCLAMPCOLORPROC_CUSTOM) (GLenum target, GLenum clamp);

static PFNGLGENFRAMEBUFFERSPROC_CUSTOM glGenFramebuffers_ptr = nullptr;
static PFNGLBINDFRAMEBUFFERPROC_CUSTOM glBindFramebuffer_ptr = nullptr;
static PFNGLFRAMEBUFFERTEXTURE2DPROC_CUSTOM glFramebufferTexture2D_ptr = nullptr;
static PFNGLGENRENDERBUFFERSPROC_CUSTOM glGenRenderbuffers_ptr = nullptr;
static PFNGLBINDRENDERBUFFERPROC_CUSTOM glBindRenderbuffer_ptr = nullptr;
static PFNGLRENDERBUFFERSTORAGEPROC_CUSTOM glRenderbufferStorage_ptr = nullptr;
static PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC_CUSTOM glRenderbufferStorageMultisample_ptr = nullptr;
static PFNGLFRAMEBUFFERRENDERBUFFERPROC_CUSTOM glFramebufferRenderbuffer_ptr = nullptr;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC_CUSTOM glCheckFramebufferStatus_ptr = nullptr;
static PFNGLDELETEFRAMEBUFFERSPROC_CUSTOM glDeleteFramebuffers_ptr = nullptr;
static PFNGLDELETERENDERBUFFERSPROC_CUSTOM glDeleteRenderbuffers_ptr = nullptr;
static PFNGLTEXIMAGE2DMULTISAMPLEPROC_CUSTOM glTexImage2DMultisample_ptr = nullptr;
static PFNGLBLITFRAMEBUFFERPROC_CUSTOM glBlitFramebuffer_ptr = nullptr;

static PFNGLCREATESHADERPROC_CUSTOM glCreateShader_ptr = nullptr;
static PFNGLSHADERSOURCEPROC_CUSTOM glShaderSource_ptr = nullptr;
static PFNGLCOMPILESHADERPROC_CUSTOM glCompileShader_ptr = nullptr;
static PFNGLGETSHADERIVPROC_CUSTOM glGetShaderiv_ptr = nullptr;
static PFNGLGETSHADERINFOLOGPROC_CUSTOM glGetShaderInfoLog_ptr = nullptr;
static PFNGLCREATEPROGRAMPROC_CUSTOM glCreateProgram_ptr = nullptr;
static PFNGLATTACHSHADERPROC_CUSTOM glAttachShader_ptr = nullptr;
static PFNGLLINKPROGRAMPROC_CUSTOM glLinkProgram_ptr = nullptr;
static PFNGLGETPROGRAMIVPROC_CUSTOM glGetProgramiv_ptr = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC_CUSTOM glGetProgramInfoLog_ptr = nullptr;
static PFNGLUSEPROGRAMPROC_CUSTOM glUseProgram_ptr = nullptr;
static PFNGLDELETEPROGRAMPROC_CUSTOM glDeleteProgram_ptr = nullptr;
static PFNGLDELETESHADERPROC_CUSTOM glDeleteShader_ptr = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC_CUSTOM glGetUniformLocation_ptr = nullptr;
static PFNGLUNIFORM1IPROC_CUSTOM glUniform1i_ptr = nullptr;
static PFNGLUNIFORM1FPROC_CUSTOM glUniform1f_ptr = nullptr;
static PFNGLUNIFORM2FPROC_CUSTOM glUniform2f_ptr = nullptr;
static PFNGLUNIFORM3FPROC_CUSTOM glUniform3f_ptr = nullptr;
static PFNGLUNIFORMMATRIX4FVPROC_CUSTOM glUniformMatrix4fv_ptr = nullptr;
static PFNGLACTIVETEXTUREPROC_CUSTOM glActiveTexture_ptr = nullptr;
static PFNGLCLAMPCOLORPROC_CUSTOM glClampColor_ptr = nullptr;

#define glGenFramebuffers glGenFramebuffers_ptr
#define glBindFramebuffer glBindFramebuffer_ptr
#define glFramebufferTexture2D glFramebufferTexture2D_ptr
#define glGenRenderbuffers glGenRenderbuffers_ptr
#define glBindRenderbuffer glBindRenderbuffer_ptr
#define glRenderbufferStorage glRenderbufferStorage_ptr
#define glRenderbufferStorageMultisample glRenderbufferStorageMultisample_ptr
#define glFramebufferRenderbuffer glFramebufferRenderbuffer_ptr
#define glCheckFramebufferStatus glCheckFramebufferStatus_ptr
#define glDeleteFramebuffers glDeleteFramebuffers_ptr
#define glDeleteRenderbuffers glDeleteRenderbuffers_ptr
#define glTexImage2DMultisample glTexImage2DMultisample_ptr
#define glBlitFramebuffer glBlitFramebuffer_ptr

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
#define glUniform1i glUniform1i_ptr
#define glUniform1f glUniform1f_ptr
#define glUniform2f glUniform2f_ptr
#define glUniform3f glUniform3f_ptr
#define glUniformMatrix4fv glUniformMatrix4fv_ptr
#define glActiveTexture glActiveTexture_ptr
#define glClampColor glClampColor_ptr

#ifndef GL_CLAMP_VERTEX_COLOR
#define GL_CLAMP_VERTEX_COLOR 0x891A
#endif
#ifndef GL_CLAMP_FRAGMENT_COLOR
#define GL_CLAMP_FRAGMENT_COLOR 0x891B
#endif

#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#endif

#ifndef GL_RGBA16F
#define GL_RGBA16F 0x881A
#endif

#ifndef GL_HALF_FLOAT
#define GL_HALF_FLOAT 0x140B
#endif

// Eight bits per channel cannot hold anything above white, so everything the
// arena draws arrives at the bright pass already flattened against the ceiling
// and no amount of strength gets more light out of it. A float target keeps
// the overshoot, which is what makes one thing glow harder than another.
//! Marks what the glow is not allowed to see.
//!
//! Turning the bikes and their trails out of the bloom cannot be done by
//! dimming them: a saturated cyan is over the threshold on its own, and the
//! answer to "no glow on my bike" is not "a duller bike". So they stamp
//! themselves into the stencil while the arena is drawn, and one pass
//! afterwards writes a nought into the alpha channel wherever the stamp is.
//! The bright pass reads that and looks straight past them.
bool rc_BloomStencilOn()
{
    extern bool sg_cycleBloom;
    extern bool sg_postProcessingEnabled;
    return sg_postProcessingEnabled && !sg_cycleBloom;
}

void rc_BloomExclude( bool on )
{
    if ( !rc_BloomStencilOn() )
        return;

    if ( on )
    {
        glEnable( GL_STENCIL_TEST );
        glStencilFunc( GL_ALWAYS, 1, 0xFF );
        glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
        glStencilMask( 0xFF );
    }
    else
    {
        glStencilMask( 0x00 );
        glDisable( GL_STENCIL_TEST );
    }
}

static GLenum s_sceneFormat = GL_RGBA16F;
static GLenum s_sceneType = GL_HALF_FLOAT;
#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif
#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif
#ifndef GL_RENDERBUFFER
#define GL_RENDERBUFFER 0x8D41
#endif
#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0 0x8CE0
#endif
#ifndef GL_DEPTH_ATTACHMENT
#define GL_DEPTH_ATTACHMENT 0x8D00
#endif
#ifndef GL_DEPTH_STENCIL_ATTACHMENT
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#endif
#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8 0x88F0
#endif
#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24 0x81A6
#endif
#ifndef GL_DEPTH_COMPONENT16
#define GL_DEPTH_COMPONENT16 0x81A5
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#endif
#ifndef GL_TEXTURE_2D_MULTISAMPLE
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_TEXTURE1
#define GL_TEXTURE1 0x84C1
#endif
#ifndef GL_TEXTURE2
#define GL_TEXTURE2 0x84C2
#endif
#ifndef GL_TEXTURE3
#define GL_TEXTURE3 0x84C3
#endif
#ifndef GL_TEXTURE4
#define GL_TEXTURE4 0x84C4
#endif
#ifndef GL_DEPTH_STENCIL
#define GL_DEPTH_STENCIL 0x84F9
#endif
#ifndef GL_UNSIGNED_INT_24_8
#define GL_UNSIGNED_INT_24_8 0x84FA
#endif

static void* GetGLProc(const char* name1, const char* name2 = nullptr) {
    void* proc = (void*)SDL_GL_GetProcAddress(name1);
    if (!proc && name2) {
        proc = (void*)SDL_GL_GetProcAddress(name2);
    }
    return proc;
}

void cPostProcessor::LoadGLExtensions() {
    if (glGenFramebuffers_ptr && glBindFramebuffer_ptr) return;

    glGenFramebuffers_ptr = (PFNGLGENFRAMEBUFFERSPROC_CUSTOM)GetGLProc("glGenFramebuffers", "glGenFramebuffersEXT");
    glBindFramebuffer_ptr = (PFNGLBINDFRAMEBUFFERPROC_CUSTOM)GetGLProc("glBindFramebuffer", "glBindFramebufferEXT");
    glFramebufferTexture2D_ptr = (PFNGLFRAMEBUFFERTEXTURE2DPROC_CUSTOM)GetGLProc("glFramebufferTexture2D", "glFramebufferTexture2DEXT");
    glGenRenderbuffers_ptr = (PFNGLGENRENDERBUFFERSPROC_CUSTOM)GetGLProc("glGenRenderbuffers", "glGenRenderbuffersEXT");
    glBindRenderbuffer_ptr = (PFNGLBINDRENDERBUFFERPROC_CUSTOM)GetGLProc("glBindRenderbuffer", "glBindRenderbufferEXT");
    glRenderbufferStorage_ptr = (PFNGLRENDERBUFFERSTORAGEPROC_CUSTOM)GetGLProc("glRenderbufferStorage", "glRenderbufferStorageEXT");
    glRenderbufferStorageMultisample_ptr = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC_CUSTOM)GetGLProc("glRenderbufferStorageMultisample", "glRenderbufferStorageMultisampleEXT");
    glFramebufferRenderbuffer_ptr = (PFNGLFRAMEBUFFERRENDERBUFFERPROC_CUSTOM)GetGLProc("glFramebufferRenderbuffer", "glFramebufferRenderbufferEXT");
    glCheckFramebufferStatus_ptr = (PFNGLCHECKFRAMEBUFFERSTATUSPROC_CUSTOM)GetGLProc("glCheckFramebufferStatus", "glCheckFramebufferStatusEXT");
    glDeleteFramebuffers_ptr = (PFNGLDELETEFRAMEBUFFERSPROC_CUSTOM)GetGLProc("glDeleteFramebuffers", "glDeleteFramebuffersEXT");
    glDeleteRenderbuffers_ptr = (PFNGLDELETERENDERBUFFERSPROC_CUSTOM)GetGLProc("glDeleteRenderbuffers", "glDeleteRenderbuffersEXT");
    glTexImage2DMultisample_ptr = (PFNGLTEXIMAGE2DMULTISAMPLEPROC_CUSTOM)GetGLProc("glTexImage2DMultisample");
    glBlitFramebuffer_ptr = (PFNGLBLITFRAMEBUFFERPROC_CUSTOM)GetGLProc("glBlitFramebuffer", "glBlitFramebufferEXT");

    glCreateShader_ptr = (PFNGLCREATESHADERPROC_CUSTOM)GetGLProc("glCreateShader");
    glShaderSource_ptr = (PFNGLSHADERSOURCEPROC_CUSTOM)GetGLProc("glShaderSource");
    glCompileShader_ptr = (PFNGLCOMPILESHADERPROC_CUSTOM)GetGLProc("glCompileShader");
    glGetShaderiv_ptr = (PFNGLGETSHADERIVPROC_CUSTOM)GetGLProc("glGetShaderiv");
    glGetShaderInfoLog_ptr = (PFNGLGETSHADERINFOLOGPROC_CUSTOM)GetGLProc("glGetShaderInfoLog");
    glCreateProgram_ptr = (PFNGLCREATEPROGRAMPROC_CUSTOM)GetGLProc("glCreateProgram");
    glAttachShader_ptr = (PFNGLATTACHSHADERPROC_CUSTOM)GetGLProc("glAttachShader");
    glLinkProgram_ptr = (PFNGLLINKPROGRAMPROC_CUSTOM)GetGLProc("glLinkProgram");
    glGetProgramiv_ptr = (PFNGLGETPROGRAMIVPROC_CUSTOM)GetGLProc("glGetProgramiv");
    glGetProgramInfoLog_ptr = (PFNGLGETPROGRAMINFOLOGPROC_CUSTOM)GetGLProc("glGetProgramInfoLog");
    glUseProgram_ptr = (PFNGLUSEPROGRAMPROC_CUSTOM)GetGLProc("glUseProgram");
    glDeleteProgram_ptr = (PFNGLDELETEPROGRAMPROC_CUSTOM)GetGLProc("glDeleteProgram");
    glDeleteShader_ptr = (PFNGLDELETESHADERPROC_CUSTOM)GetGLProc("glDeleteShader");
    glGetUniformLocation_ptr = (PFNGLGETUNIFORMLOCATIONPROC_CUSTOM)GetGLProc("glGetUniformLocation");
    glUniform1i_ptr = (PFNGLUNIFORM1IPROC_CUSTOM)GetGLProc("glUniform1i");
    glUniform1f_ptr = (PFNGLUNIFORM1FPROC_CUSTOM)GetGLProc("glUniform1f");
    glUniform2f_ptr = (PFNGLUNIFORM2FPROC_CUSTOM)GetGLProc("glUniform2f");
    glUniform3f_ptr = (PFNGLUNIFORM3FPROC_CUSTOM)GetGLProc("glUniform3f");
    glUniformMatrix4fv_ptr = (PFNGLUNIFORMMATRIX4FVPROC_CUSTOM)GetGLProc("glUniformMatrix4fv");
    glActiveTexture_ptr = (PFNGLACTIVETEXTUREPROC_CUSTOM)GetGLProc("glActiveTexture");
}

cPostProcessor::cPostProcessor()
    : m_initialized(false)
    , m_fboComplete(false)
    , m_useMSAA(false)
    , m_isBound(false)
    , m_width(0)
    , m_height(0)
    , m_msaaFBO(0)
    , m_msaaColorTexture(0)
    , m_msaaDepthRBO(0)
    , m_resolveFBO(0)
    , m_resolveColorTexture(0)
    , m_resolveDepthRBO(0)
    , m_shaderProgram(0)
    , m_uTextureLoc(-1)
    , m_uResolutionLoc(-1)
    , m_uVignetteIntensityLoc(-1)
    , m_uVignetteRadiusLoc(-1)
    , m_uVignetteSoftnessLoc(-1)
    , m_uBloomThresholdLoc(-1)
    , m_uBloomTextureLoc(-1)
    , m_uBloomStrengthLoc(-1)
{
    for ( int i = 0; i < kBloomLevels; ++i )
    {
        m_bloomFBO[i] = m_bloomTex[i] = m_bloomTempFBO[i] = m_bloomTempTex[i] = 0;
        m_bloomW[i] = m_bloomH[i] = 0;
    }
    m_bloomReady = false;
    m_brightProgram = 0;
    m_uBrightTexture = m_uBrightThreshold = -1;
    m_blurProgram = 0;
    m_uBlurTexture = m_uBlurDirection = -1;
    m_downProgram = 0;
    m_uDownTexture = m_uDownTexel = m_uDownKaris = -1;

}

cPostProcessor::~cPostProcessor() {
    Cleanup();
}

void cPostProcessor::CleanupFBO() {
    if (m_msaaColorTexture) {
        glDeleteTextures(1, &m_msaaColorTexture);
        m_msaaColorTexture = 0;
    }
    if (m_msaaDepthRBO && glDeleteRenderbuffers) {
        glDeleteRenderbuffers(1, &m_msaaDepthRBO);
        m_msaaDepthRBO = 0;
    }
    if (m_msaaFBO && glDeleteFramebuffers) {
        glDeleteFramebuffers(1, &m_msaaFBO);
        m_msaaFBO = 0;
    }

    if (m_resolveColorTexture) {
        glDeleteTextures(1, &m_resolveColorTexture);
        m_resolveColorTexture = 0;
    }
    if (m_resolveDepthRBO && glDeleteRenderbuffers) {
        glDeleteRenderbuffers(1, &m_resolveDepthRBO);
        m_resolveDepthRBO = 0;
    }
    if (m_resolveFBO && glDeleteFramebuffers) {
        glDeleteFramebuffers(1, &m_resolveFBO);
        m_resolveFBO = 0;
    }

    DestroyBloomTargets();

    m_initialized = false;
    m_fboComplete = false;
    m_useMSAA = false;
}

void cPostProcessor::Cleanup() {
    CleanupFBO();
    if (m_shaderProgram && glDeleteProgram) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
    if (m_brightProgram && glDeleteProgram) {
        glDeleteProgram(m_brightProgram);
        m_brightProgram = 0;
    }
    if (m_blurProgram && glDeleteProgram) {
        glDeleteProgram(m_blurProgram);
        m_blurProgram = 0;
    }
    if (m_downProgram && glDeleteProgram) {
        glDeleteProgram(m_downProgram);
        m_downProgram = 0;
    }
}

void cPostProcessor::CompileShaders() {
    const char* vertexSource =
        "#version 120\n"
        "varying vec2 TexCoords;\n"
        "void main() {\n"
        "    TexCoords = gl_MultiTexCoord0.st;\n"
        "    gl_Position = vec4(gl_Vertex.xy, 0.0, 1.0);\n"
        "}\n";

    const char* fragmentSource =
        "#version 120\n"
        "varying vec2 TexCoords;\n"
        "uniform sampler2D screenTexture;\n"
        "uniform sampler2D bloomTexture;\n"
        "uniform float u_bloomStrength;\n"
        "uniform float u_bloomThreshold;\n"
        "uniform float u_vignetteIntensity;\n"
        "uniform float u_vignetteRadius;\n"
        "uniform float u_vignetteSoftness;\n"
        "uniform vec2 u_resolution;\n"
        "vec3 ACESFilm(vec3 x) {\n"
        "    float a = 2.51;\n"
        "    float b = 0.03;\n"
        "    float c = 2.43;\n"
        "    float d = 0.59;\n"
        "    float e = 0.14;\n"
        "    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);\n"
        "}\n"
        "void main() {\n"
        "    vec4 sceneColor = texture2D(screenTexture, TexCoords);\n"
        "    vec3 rgb = sceneColor.rgb;\n"
        "    vec3 bloom = texture2D(bloomTexture, TexCoords).rgb;\n"
        // Tonemapping the sum would spend the whole exposure on the scene and
        // hand the glow back flattened. The light goes on top of the graded
        // image instead, which is the point of it being light.
        "    vec3 tonemapped = ACESFilm(rgb * 1.1);\n"
        "    tonemapped += ACESFilm(bloom * u_bloomStrength) * 0.85;\n"
        "    vec2 uv = TexCoords - vec2(0.5);\n"
        "    uv.x *= u_resolution.x / max(u_resolution.y, 1.0);\n"
        "    float dist = length(uv);\n"
        "    float vignette = smoothstep(u_vignetteRadius, u_vignetteRadius - u_vignetteSoftness, dist);\n"
        "    vec3 finalColor = mix(tonemapped * vignette, tonemapped, 1.0 - u_vignetteIntensity);\n"
        "    gl_FragColor = vec4(finalColor, sceneColor.a);\n"
        "}\n";

    if (!glCreateShader || !glShaderSource || !glCompileShader || !glCreateProgram) {
        std::cerr << "[cPostProcessor] Shader creation functions not available." << std::endl;
        return;
    }

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexSource, nullptr);
    glCompileShader(vs);

    GLint success = 0;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vs, 512, nullptr, infoLog);
        std::cerr << "[cPostProcessor] Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentSource, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fs, 512, nullptr, infoLog);
        std::cerr << "[cPostProcessor] Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vs);
    glAttachShader(m_shaderProgram, fs);
    glLinkProgram(m_shaderProgram);

    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
        std::cerr << "[cPostProcessor] Shader Program Link Failed:\n" << infoLog << std::endl;
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (m_shaderProgram) {
        m_uTextureLoc = glGetUniformLocation(m_shaderProgram, "screenTexture");
        m_uResolutionLoc = glGetUniformLocation(m_shaderProgram, "u_resolution");
        m_uVignetteIntensityLoc = glGetUniformLocation(m_shaderProgram, "u_vignetteIntensity");
        m_uVignetteRadiusLoc = glGetUniformLocation(m_shaderProgram, "u_vignetteRadius");
        m_uVignetteSoftnessLoc = glGetUniformLocation(m_shaderProgram, "u_vignetteSoftness");
        m_uBloomThresholdLoc = glGetUniformLocation(m_shaderProgram, "u_bloomThreshold");
        m_uBloomTextureLoc = glGetUniformLocation(m_shaderProgram, "bloomTexture");
        m_uBloomStrengthLoc = glGetUniformLocation(m_shaderProgram, "u_bloomStrength");
    }
}

void cPostProcessor::Init(int width, int height) {
    if (width <= 0 || height <= 0) return;
    LoadGLExtensions();

    if (!glGenFramebuffers || !glBindFramebuffer) {
        m_fboComplete = false;
        m_initialized = false;
        return;
    }

    if (m_initialized) {
        if (m_width == width && m_height == height) return;
        CleanupFBO();
    }

    m_width = width;
    m_height = height;

    // STEP 1: Create Resolve FBO (Standard 2D HDR)
    glGenFramebuffers(1, &m_resolveFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_resolveFBO);

    glGenTextures(1, &m_resolveColorTexture);
    glBindTexture(GL_TEXTURE_2D, m_resolveColorTexture);
    // whatever the arena left in the error queue is not ours to read
    while (glGetError() != GL_NO_ERROR) {}

    glTexImage2D(GL_TEXTURE_2D, 0, s_sceneFormat, m_width, m_height, 0, GL_RGBA, s_sceneType, nullptr);
    if (glGetError() != GL_NO_ERROR) {
        // no float textures here; fall back and lose the overshoot
        s_sceneFormat = GL_RGBA8;
        s_sceneType = GL_UNSIGNED_BYTE;
        glTexImage2D(GL_TEXTURE_2D, 0, s_sceneFormat, m_width, m_height, 0, GL_RGBA, s_sceneType, nullptr);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F); // GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resolveColorTexture, 0);

    glGenRenderbuffers(1, &m_resolveDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_resolveDepthRBO);
    if (glRenderbufferStorage) {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_resolveDepthRBO);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width, m_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_resolveDepthRBO);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_resolveDepthRBO);
        }
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[cPostProcessor] Resolve FBO status check failed!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_fboComplete = false;
        m_initialized = true;
        return;
    }

    // STEP 2: Try creating MSAA FBO if extensions available
    m_useMSAA = false;
    if (glTexImage2DMultisample && glRenderbufferStorageMultisample && glBlitFramebuffer) {
        glGenFramebuffers(1, &m_msaaFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFBO);

        glGenTextures(1, &m_msaaColorTexture);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture);
        // Eight bit samples here would throw the overshoot away before the
        // resolve ever sees it, and the float target downstream would be
        // holding a picture that has already been flattened. Ask for the same
        // format the rest of the chain works in, and only settle for less if
        // the card will not give it.
        while (glGetError() != GL_NO_ERROR) {}
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, s_sceneFormat, m_width, m_height, GL_TRUE);
        if (glGetError() != GL_NO_ERROR) {
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 8, GL_RGBA8, m_width, m_height, GL_TRUE);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_msaaColorTexture, 0);

        glGenRenderbuffers(1, &m_msaaDepthRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_msaaDepthRBO);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_DEPTH24_STENCIL8, m_width, m_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_msaaDepthRBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
            m_useMSAA = true;
        } else {
            std::cerr << "[cPostProcessor] MSAA FBO incomplete, using Standard 2D FBO." << std::endl;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (!m_shaderProgram) {
        CompileShaders();
    }

    BuildBloomTargets();

    m_fboComplete = true;
    m_initialized = true;
}

namespace
{

//! Compiles one throwaway program out of a vertex and a fragment source.
GLuint BuildProgram( char const * vertexSource, char const * fragmentSource, char const * what )
{
    if ( !glCreateShader || !glShaderSource || !glCompileShader || !glCreateProgram )
        return 0;

    GLint ok = 0;
    char log[512];

    GLuint vs = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vs, 1, &vertexSource, nullptr );
    glCompileShader( vs );
    glGetShaderiv( vs, GL_COMPILE_STATUS, &ok );
    if ( !ok )
    {
        glGetShaderInfoLog( vs, 512, nullptr, log );
        std::cerr << "[cPostProcessor] " << what << " vertex stage failed:\n" << log << std::endl;
    }

    GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fs, 1, &fragmentSource, nullptr );
    glCompileShader( fs );
    glGetShaderiv( fs, GL_COMPILE_STATUS, &ok );
    if ( !ok )
    {
        glGetShaderInfoLog( fs, 512, nullptr, log );
        std::cerr << "[cPostProcessor] " << what << " fragment stage failed:\n" << log << std::endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader( program, vs );
    glAttachShader( program, fs );
    glLinkProgram( program );
    glGetProgramiv( program, GL_LINK_STATUS, &ok );
    if ( !ok )
    {
        glGetProgramInfoLog( program, 512, nullptr, log );
        std::cerr << "[cPostProcessor] " << what << " link failed:\n" << log << std::endl;
        glDeleteProgram( program );
        program = 0;
    }

    glDeleteShader( vs );
    glDeleteShader( fs );
    return program;
}

char const * const kPassVertex =
    "#version 120\n"
    "varying vec2 TexCoords;\n"
    "void main() {\n"
    "    TexCoords = gl_MultiTexCoord0.st;\n"
    "    gl_Position = vec4(gl_Vertex.xy, 0.0, 1.0);\n"
    "}\n";

// Keeps the colour, scales it by how far past the threshold it is. The knee
// stops a pixel from snapping into the glow the moment it crosses.
char const * const kBrightFragment =
    "#version 120\n"
    "varying vec2 TexCoords;\n"
    "uniform sampler2D srcTexture;\n"
    "uniform float u_threshold;\n"
    "void main() {\n"
    "    vec4 src = texture2D(srcTexture, TexCoords);\n"
    "    if (src.a < 0.5) { gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); return; }\n"
    "    vec3 c = src.rgb;\n"
    "    float lum = dot(c, vec3(0.2126, 0.7152, 0.0722));\n"
    "    float knee = 0.18;\n"
    "    float soft = clamp(lum - u_threshold + knee, 0.0, 2.0 * knee);\n"
    "    soft = soft * soft / (4.0 * knee + 0.0001);\n"
    "    float bright = max(lum - u_threshold, soft);\n"
    "    float w = lum > 0.0001 ? bright / lum : 0.0;\n"
    "    gl_FragColor = vec4(c * clamp(w, 0.0, 1.0), 1.0);\n"
    "}\n";

// Nine taps along whichever axis the direction points at.
// Halving an image by just sampling it is what makes bloom crawl: a bright
// line one pixel wide either lands on a sample or falls between two, and which
// one it does changes as you walk towards it. Thirteen taps arranged in a
// square and a cross read the whole neighbourhood instead, so the line keeps
// the same weight wherever it sits. The first rung also averages by luminance
// rather than plainly, which stops a single very bright pixel from pumping the
// whole halo on and off.
char const * const kDownFragment =
    "#version 120\n"
    "varying vec2 TexCoords;\n"
    "uniform sampler2D srcTexture;\n"
    "uniform vec2 u_texel;\n"
    "uniform float u_karis;\n"
    "vec3 tap(vec2 o) { return texture2D(srcTexture, TexCoords + o).rgb; }\n"
    "float weigh(vec3 c) { return 1.0 / (1.0 + dot(c, vec3(0.2126, 0.7152, 0.0722))); }\n"
    "vec3 group(vec3 a, vec3 b, vec3 c, vec3 d) {\n"
    "    if (u_karis < 0.5) return (a + b + c + d) * 0.25;\n"
    "    float wa = weigh(a), wb = weigh(b), wc = weigh(c), wd = weigh(d);\n"
    "    return (a*wa + b*wb + c*wc + d*wd) / max(wa + wb + wc + wd, 0.0001);\n"
    "}\n"
    "void main() {\n"
    "    vec2 t = u_texel;\n"
    "    vec3 a = tap(vec2(-2.0, 2.0)*t), b = tap(vec2(0.0, 2.0)*t), c = tap(vec2(2.0, 2.0)*t);\n"
    "    vec3 d = tap(vec2(-2.0, 0.0)*t), e = tap(vec2(0.0, 0.0)*t), f = tap(vec2(2.0, 0.0)*t);\n"
    "    vec3 g = tap(vec2(-2.0,-2.0)*t), h = tap(vec2(0.0,-2.0)*t), i = tap(vec2(2.0,-2.0)*t);\n"
    "    vec3 j = tap(vec2(-1.0, 1.0)*t), k = tap(vec2(1.0, 1.0)*t);\n"
    "    vec3 l = tap(vec2(-1.0,-1.0)*t), m = tap(vec2(1.0,-1.0)*t);\n"
    "    vec3 sum = group(j, k, l, m) * 0.5;\n"
    "    sum += group(a, b, d, e) * 0.125;\n"
    "    sum += group(b, c, e, f) * 0.125;\n"
    "    sum += group(d, e, g, h) * 0.125;\n"
    "    sum += group(e, f, h, i) * 0.125;\n"
    "    gl_FragColor = vec4(sum, 1.0);\n"
    "}\n";

char const * const kBlurFragment =
    "#version 120\n"
    "varying vec2 TexCoords;\n"
    "uniform sampler2D srcTexture;\n"
    "uniform vec2 u_direction;\n"
    "void main() {\n"
    "    vec3 r = texture2D(srcTexture, TexCoords).rgb * 0.227027;\n"
    "    r += texture2D(srcTexture, TexCoords + u_direction * 1.0).rgb * 0.1945946;\n"
    "    r += texture2D(srcTexture, TexCoords - u_direction * 1.0).rgb * 0.1945946;\n"
    "    r += texture2D(srcTexture, TexCoords + u_direction * 2.0).rgb * 0.1216216;\n"
    "    r += texture2D(srcTexture, TexCoords - u_direction * 2.0).rgb * 0.1216216;\n"
    "    r += texture2D(srcTexture, TexCoords + u_direction * 3.0).rgb * 0.0540540;\n"
    "    r += texture2D(srcTexture, TexCoords - u_direction * 3.0).rgb * 0.0540540;\n"
    "    r += texture2D(srcTexture, TexCoords + u_direction * 4.0).rgb * 0.0162162;\n"
    "    r += texture2D(srcTexture, TexCoords - u_direction * 4.0).rgb * 0.0162162;\n"
    "    gl_FragColor = vec4(r, 1.0);\n"
    "}\n";

}

void cPostProcessor::DestroyBloomTargets()
{
    for ( int i = 0; i < kBloomLevels; ++i )
    {
        if ( m_bloomTex[i] )     { glDeleteTextures( 1, &m_bloomTex[i] );     m_bloomTex[i] = 0; }
        if ( m_bloomTempTex[i] ) { glDeleteTextures( 1, &m_bloomTempTex[i] ); m_bloomTempTex[i] = 0; }
        if ( m_bloomFBO[i] && glDeleteFramebuffers )     { glDeleteFramebuffers( 1, &m_bloomFBO[i] );     m_bloomFBO[i] = 0; }
        if ( m_bloomTempFBO[i] && glDeleteFramebuffers ) { glDeleteFramebuffers( 1, &m_bloomTempFBO[i] ); m_bloomTempFBO[i] = 0; }
        m_bloomW[i] = m_bloomH[i] = 0;
    }
    m_bloomReady = false;
}

void cPostProcessor::BuildBloomTargets()
{
    DestroyBloomTargets();

    if ( !glGenFramebuffers || !glBindFramebuffer || !glFramebufferTexture2D )
        return;

    int w = m_width, h = m_height;
    for ( int i = 0; i < kBloomLevels; ++i )
    {
        w = w > 1 ? w / 2 : 1;
        h = h > 1 ? h / 2 : 1;
        m_bloomW[i] = w;
        m_bloomH[i] = h;

        GLuint * tex[2] = { &m_bloomTex[i], &m_bloomTempTex[i] };
        GLuint * fbo[2] = { &m_bloomFBO[i], &m_bloomTempFBO[i] };

        for ( int k = 0; k < 2; ++k )
        {
            glGenTextures( 1, tex[k] );
            glBindTexture( GL_TEXTURE_2D, *tex[k] );
            glTexImage2D( GL_TEXTURE_2D, 0, s_sceneFormat, w, h, 0, GL_RGBA, s_sceneType, nullptr );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F );

            glGenFramebuffers( 1, fbo[k] );
            glBindFramebuffer( GL_FRAMEBUFFER, *fbo[k] );
            glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tex[k], 0 );

            if ( glCheckFramebufferStatus && glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
            {
                std::cerr << "[cPostProcessor] bloom target " << i << " incomplete." << std::endl;
                glBindFramebuffer( GL_FRAMEBUFFER, 0 );
                DestroyBloomTargets();
                return;
            }
        }
    }

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    if ( !m_brightProgram )
    {
        m_brightProgram = BuildProgram( kPassVertex, kBrightFragment, "bright pass" );
        if ( m_brightProgram )
        {
            m_uBrightTexture = glGetUniformLocation( m_brightProgram, "srcTexture" );
            m_uBrightThreshold = glGetUniformLocation( m_brightProgram, "u_threshold" );
        }
    }

    if ( !m_downProgram )
    {
        m_downProgram = BuildProgram( kPassVertex, kDownFragment, "downsample" );
        if ( m_downProgram )
        {
            m_uDownTexture = glGetUniformLocation( m_downProgram, "srcTexture" );
            m_uDownTexel = glGetUniformLocation( m_downProgram, "u_texel" );
            m_uDownKaris = glGetUniformLocation( m_downProgram, "u_karis" );
        }
    }

    if ( !m_blurProgram )
    {
        m_blurProgram = BuildProgram( kPassVertex, kBlurFragment, "blur" );
        if ( m_blurProgram )
        {
            m_uBlurTexture = glGetUniformLocation( m_blurProgram, "srcTexture" );
            m_uBlurDirection = glGetUniformLocation( m_blurProgram, "u_direction" );
        }
    }

    m_bloomReady = ( m_brightProgram != 0 && m_blurProgram != 0 && m_downProgram != 0 );
}

void cPostProcessor::DrawInto( GLuint fbo, int w, int h )
{
    glBindFramebuffer( GL_FRAMEBUFFER, fbo );
    glViewport( 0, 0, w, h );
    RenderQuadInternal();
}

void cPostProcessor::RunBloom()
{
    if ( !m_bloomReady )
        return;

    glDisable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
    if ( glActiveTexture )
        glActiveTexture( GL_TEXTURE0 );

    // the copies between levels go through the fixed pipeline, which multiplies
    // by whatever colour the arena happened to leave behind
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

    // the bright parts of the frame, at half size
    glUseProgram( m_brightProgram );
    glBindTexture( GL_TEXTURE_2D, m_resolveColorTexture );
    if ( m_uBrightTexture != -1 )   glUniform1i( m_uBrightTexture, 0 );
    if ( m_uBrightThreshold != -1 ) glUniform1f( m_uBrightThreshold, sg_bloomThreshold );
    DrawInto( m_bloomFBO[0], m_bloomW[0], m_bloomH[0] );

    // the ladder down, each rung half the one above it
    glUseProgram( m_downProgram );
    if ( m_uDownTexture != -1 )
        glUniform1i( m_uDownTexture, 0 );

    for ( int i = 1; i < kBloomLevels; ++i )
    {
        glBindTexture( GL_TEXTURE_2D, m_bloomTex[i-1] );
        if ( m_uDownTexel != -1 )
            glUniform2f( m_uDownTexel, 1.0f / REAL( m_bloomW[i-1] ), 1.0f / REAL( m_bloomH[i-1] ) );
        if ( m_uDownKaris != -1 )
            glUniform1f( m_uDownKaris, i == 1 ? 1.0f : 0.0f );
        DrawInto( m_bloomFBO[i], m_bloomW[i], m_bloomH[i] );
    }

    // and back up: blur each rung, add it into the one above
    glUseProgram( m_blurProgram );
    if ( m_uBlurTexture != -1 )
        glUniform1i( m_uBlurTexture, 0 );

    for ( int i = kBloomLevels - 1; i >= 0; --i )
    {
        REAL sx = sg_bloomSpread / REAL( m_bloomW[i] );
        REAL sy = sg_bloomSpread / REAL( m_bloomH[i] );

        glBindTexture( GL_TEXTURE_2D, m_bloomTex[i] );
        if ( m_uBlurDirection != -1 ) glUniform2f( m_uBlurDirection, sx, 0.0f );
        DrawInto( m_bloomTempFBO[i], m_bloomW[i], m_bloomH[i] );

        glBindTexture( GL_TEXTURE_2D, m_bloomTempTex[i] );
        if ( m_uBlurDirection != -1 ) glUniform2f( m_uBlurDirection, 0.0f, sy );
        DrawInto( m_bloomFBO[i], m_bloomW[i], m_bloomH[i] );

        if ( i > 0 )
        {
            glUseProgram( 0 );
            glEnable( GL_BLEND );
            glBlendFunc( GL_ONE, GL_ONE );
            glBindTexture( GL_TEXTURE_2D, m_bloomTex[i] );
            DrawInto( m_bloomFBO[i-1], m_bloomW[i-1], m_bloomH[i-1] );
            glDisable( GL_BLEND );
            glUseProgram( m_blurProgram );
            if ( m_uBlurTexture != -1 )
                glUniform1i( m_uBlurTexture, 0 );
        }
    }

    glUseProgram( 0 );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glViewport( 0, 0, sr_screenWidth, sr_screenHeight );
}

void cPostProcessor::BindMSAAFBO() {
    // Light does not belong to the enhanced renderer any more than shadow
    // does. It used to be locked behind that switch, which meant turning the
    // neon grid off took the bloom with it.
    if (!sg_postProcessingEnabled) return;

    Init(sr_screenWidth, sr_screenHeight);

    if (!m_initialized || !m_fboComplete) return;

    GLuint targetFBO = (m_useMSAA && m_msaaFBO) ? m_msaaFBO : m_resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.02f, 0.02f, 0.04f, 1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // The fixed pipeline folds every colour back to white on its way out. That
    // is the right thing for an eight bit screen and exactly wrong here: the
    // overshoot is the whole point of the float target.
    if ( glClampColor && s_sceneFormat != GL_RGBA8 )
    {
        glClampColor( GL_CLAMP_VERTEX_COLOR, GL_FALSE );
        glClampColor( GL_CLAMP_FRAGMENT_COLOR, GL_FALSE );
    }

    m_isBound = true;
}

void cPostProcessor::UnbindFBO() {
    if (!m_isBound) {
        if (glBindFramebuffer) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        return;
    }
    m_isBound = false;

    if (!sg_postProcessingEnabled || !m_initialized || !m_fboComplete) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // whatever carries the stamp loses its alpha, which is what the bright
    // pass looks at
    if ( rc_BloomStencilOn() )
    {
        glEnable( GL_STENCIL_TEST );
        glStencilFunc( GL_EQUAL, 1, 0xFF );
        glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
        glStencilMask( 0x00 );

        glDisable( GL_DEPTH_TEST );
        glDisable( GL_BLEND );
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE );

        glMatrixMode( GL_PROJECTION ); glPushMatrix(); glLoadIdentity();
        glMatrixMode( GL_MODELVIEW );  glPushMatrix(); glLoadIdentity();
        glColor4f( 0, 0, 0, 0 );
        RenderQuadInternal();
        glMatrixMode( GL_PROJECTION ); glPopMatrix();
        glMatrixMode( GL_MODELVIEW );  glPopMatrix();

        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
        glDisable( GL_STENCIL_TEST );
    }

    // Hardware Blit MSAA FBO -> Resolve FBO if MSAA enabled
    if (m_useMSAA && glBlitFramebuffer && m_msaaFBO && m_resolveFBO) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolveFBO);
        glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    if ( glClampColor && s_sceneFormat != GL_RGBA8 )
    {
        glClampColor( GL_CLAMP_VERTEX_COLOR, GL_TRUE );
        glClampColor( GL_CLAMP_FRAGMENT_COLOR, GL_TRUE );
    }

    // Return render target to default monitor backbuffer (0)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_width, m_height);
}

void cPostProcessor::RenderQuadInternal() {
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
    glEnd();
}

void cPostProcessor::RenderFullscreenQuad() {
    if (!sg_postProcessingEnabled || !m_initialized || !m_fboComplete) return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glViewport(0, 0, sr_screenWidth, sr_screenHeight);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    RunBloom();

    if (m_shaderProgram && glUseProgram) {
        glUseProgram(m_shaderProgram);

        if (glActiveTexture && m_bloomReady) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_bloomTex[0]);
            if (m_uBloomTextureLoc != -1) glUniform1i(m_uBloomTextureLoc, 1);
        }
        if (m_uBloomStrengthLoc != -1)
            glUniform1f(m_uBloomStrengthLoc, m_bloomReady ? sg_bloomStrength : 0.0f);

        if (glActiveTexture) {
            glActiveTexture(GL_TEXTURE0);
        }
        glBindTexture(GL_TEXTURE_2D, m_resolveColorTexture);
        if (m_uTextureLoc != -1) glUniform1i(m_uTextureLoc, 0);
        if (m_uResolutionLoc != -1) glUniform2f(m_uResolutionLoc, (float)m_width, (float)m_height);
        if (m_uVignetteIntensityLoc != -1) glUniform1f(m_uVignetteIntensityLoc, sg_vignetteIntensity);
        if (m_uVignetteRadiusLoc != -1) glUniform1f(m_uVignetteRadiusLoc, sg_vignetteRadius);
        if (m_uVignetteSoftnessLoc != -1) glUniform1f(m_uVignetteSoftnessLoc, sg_vignetteSoftness);
        if (m_uBloomThresholdLoc != -1) glUniform1f(m_uBloomThresholdLoc, sg_bloomThreshold);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_resolveColorTexture);
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderQuadInternal();

    if (glUseProgram) {
        glUseProgram(0);
    }

    if (glActiveTexture) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
    }

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glPopAttrib();
}

#endif // DEDICATED
