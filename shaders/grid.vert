#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 vWorldPos;
out vec2 vTexCoord;
out vec4 vClipSpace;

void main() {
    vec4 worldPos = u_model * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;
    vTexCoord = aTexCoord;
    vClipSpace = u_projection * u_view * worldPos;
    gl_Position = vClipSpace;
}
