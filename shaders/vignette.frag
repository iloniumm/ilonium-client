#version 120

uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform float u_vignetteIntensity;
uniform float u_vignetteRadius;
uniform float u_vignetteSoftness;

varying vec2 vTexCoord;

void main() {
    // Sample texture from FBO color buffer
    vec4 color = texture2D(u_texture, vTexCoord);
    
    // Normalized screen space coordinates centered at (0, 0)
    vec2 uv = vTexCoord - vec2(0.5);
    
    // Correct aspect ratio distortion
    uv.x *= u_resolution.x / u_resolution.y;
    
    // Distance from center
    float dist = length(uv);
    
    // Smooth cinematic vignette falloff
    float vignette = smoothstep(u_vignetteRadius, u_vignetteRadius - u_vignetteSoftness, dist);
    
    // Blend final RGB using u_vignetteIntensity
    vec3 finalRGB = mix(color.rgb * vignette, color.rgb, 1.0 - u_vignetteIntensity);
    
    gl_FragColor = vec4(finalRGB, color.a);
}
