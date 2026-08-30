#version 330 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float u_bloomThreshold;
uniform float u_vignetteIntensity;
uniform float u_vignetteRadius;
uniform float u_vignetteSoftness;
uniform vec2 u_resolution;

void main() {
    // 1. Sample resolved HDR 2D texture from MSAA Resolve FBO
    vec4 hdrColor = texture(screenTexture, TexCoords);
    
    // 2. Cinematic Vignette Pass
    vec2 uv = TexCoords - vec2(0.5);
    uv.x *= u_resolution.x / u_resolution.y;
    float dist = length(uv);
    float vignette = smoothstep(u_vignetteRadius, u_vignetteRadius - u_vignetteSoftness, dist);
    vec3 colorWithVignette = mix(hdrColor.rgb * vignette, hdrColor.rgb, 1.0 - u_vignetteIntensity);

    // 3. Bright Pass extraction for Bloom (HDR thresholding)
    float brightness = dot(colorWithVignette, vec3(0.2126, 0.7152, 0.0722));
    vec3 brightColor = vec3(0.0);
    if (brightness > u_bloomThreshold) {
        brightColor = colorWithVignette;
    }

    // Combine base HDR color with bright pass highlight
    FragColor = vec4(colorWithVignette + brightColor * 0.5, hdrColor.a);
}
