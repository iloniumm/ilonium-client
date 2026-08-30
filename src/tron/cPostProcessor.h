#ifndef CPOST_PROCESSOR_H
#define CPOST_PROCESSOR_H

#ifndef DEDICATED
#include "render/rGL.h"
#include "rSDL.h"
#endif

extern bool  sg_postProcessingEnabled;

//! Stamps whatever is drawn between the calls as out of bounds for the glow.
void rc_BloomExclude( bool on );
extern float sg_vignetteIntensity;
extern float sg_vignetteRadius;
extern float sg_vignetteSoftness;
extern float sg_bloomThreshold;
extern float sg_bloomStrength;
extern float sg_bloomSpread;

class cPostProcessor {
public:
    static cPostProcessor& Instance() {
        static cPostProcessor instance;
        return instance;
    }

    void Init(int width, int height);
    
    // Main Render Loop Hijack API
    void BindMSAAFBO();
    void UnbindFBO();
    void RenderFullscreenQuad();

    // Legacy Aliases
    void BeginFrame(int width, int height) { Init(width, height); BindMSAAFBO(); }
    void EndFrame() { UnbindFBO(); RenderFullscreenQuad(); }

    void Cleanup();

    bool IsBound() const { return m_isBound; }
    bool IsFBOComplete() const { return m_fboComplete; }

private:
    cPostProcessor();
    ~cPostProcessor();

    cPostProcessor(const cPostProcessor&) = delete;
    cPostProcessor& operator=(const cPostProcessor&) = delete;

    void LoadGLExtensions();
    void CompileShaders();
    void CleanupFBO();
    void RenderQuadInternal();

    // The bloom chain: a bright pass into half size, then a ladder of ever
    // smaller copies, each one blurred in both directions and added back into
    // the one above it. Blurring a small image is what buys the reach - a
    // nine tap at a sixteenth of the screen covers a hundred pixels of it,
    // which no single pass at full size could afford.
#ifndef DEDICATED
    void BuildBloomTargets();
    void DestroyBloomTargets();
    void RunBloom();
    void DrawInto( GLuint fbo, int w, int h );
#endif

    bool m_initialized;
    bool m_fboComplete;
    bool m_useMSAA;
    bool m_isBound;
    int  m_width;
    int  m_height;

#ifndef DEDICATED
    // MSAA FBO (8x Multisample)
    GLuint m_msaaFBO;
    GLuint m_msaaColorTexture;
    GLuint m_msaaDepthRBO;

    // Resolve FBO (Flat 2D HDR)
    GLuint m_resolveFBO;
    GLuint m_resolveColorTexture;
    GLuint m_resolveDepthRBO;
    static const int kBloomLevels = 5;
    GLuint m_bloomFBO[kBloomLevels];
    GLuint m_bloomTex[kBloomLevels];
    GLuint m_bloomTempFBO[kBloomLevels];
    GLuint m_bloomTempTex[kBloomLevels];
    int    m_bloomW[kBloomLevels];
    int    m_bloomH[kBloomLevels];
    bool   m_bloomReady;

    GLuint m_brightProgram;
    GLint  m_uBrightTexture;
    GLint  m_uBrightThreshold;

    GLuint m_downProgram;
    GLint  m_uDownTexture;
    GLint  m_uDownTexel;
    GLint  m_uDownKaris;

    GLuint m_blurProgram;
    GLint  m_uBlurTexture;
    GLint  m_uBlurDirection;

    // Bright Pass GLSL Shader Program
    GLuint m_shaderProgram;
    GLint  m_uTextureLoc;
    GLint  m_uResolutionLoc;
    GLint  m_uVignetteIntensityLoc;
    GLint  m_uVignetteRadiusLoc;
    GLint  m_uVignetteSoftnessLoc;
    GLint  m_uBloomThresholdLoc;
    GLint  m_uBloomTextureLoc;
    GLint  m_uBloomStrengthLoc;
#endif
};

#endif // CPOST_PROCESSOR_H
