#version 330 core

in vec3 vWorldPos;
in vec2 vTexCoord;
in vec4 vClipSpace;

out vec4 FragColor;

uniform sampler2D u_reflectionTexture;
uniform vec3 u_cameraPos;
uniform vec3 u_gridColor;
uniform float u_gridScale;
uniform float u_gridThickness;

void main() {
    // 1. Procedural Grid Line Math (Antialiased using fwidth)
    vec2 gridUV = vWorldPos.xy * u_gridScale;
    vec2 grid = abs(fract(gridUV - 0.5) - 0.5);
    vec2 lineWeight = fwidth(gridUV) * u_gridThickness;
    vec2 gridLines = smoothstep(lineWeight, vec2(0.0), grid);
    float gridIntensity = max(gridLines.x, gridLines.y);

    // 2. Radial Distance Fog / Fade
    float dist = length(vWorldPos.xy - u_cameraPos.xy);
    float fade = exp(-dist * 0.005);
    gridIntensity *= fade;

    // 3. Planar Reflection UV (Screen space NDC coordinates)
    vec2 ndc = (vClipSpace.xy / vClipSpace.w) * 0.5 + 0.5;

    // Subtle UV distortion for polished dark glass floor reflection
    vec2 distortion = vec2(
        sin(vWorldPos.y * 0.5 + vWorldPos.x * 0.2) * 0.003,
        cos(vWorldPos.x * 0.5 - vWorldPos.y * 0.2) * 0.003
    );
    vec2 reflectionUV = clamp(ndc + distortion, 0.0, 1.0);
    vec3 reflectionColor = texture(u_reflectionTexture, reflectionUV).rgb;

    // 4. Dark glass floor base color
    vec3 baseFloor = vec3(0.03, 0.05, 0.08);

    // 5. Combine reflective dark glass with glowing grid lines
    vec3 finalRGB = mix(baseFloor, reflectionColor * 0.65 + baseFloor, 0.7);
    finalRGB += u_gridColor * gridIntensity * 1.5;

    FragColor = vec4(finalRGB, 1.0);
}
