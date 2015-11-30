#version 430

uniform vec3 eyePosition;

in vec3 vPosition;
in vec3 vColor;
in vec3 vNormal;

layout(location = 0) out vec3 color;
layout(location = 1) out float depth;

float fogEnd = 100;
float fogStart = 75;

void main() {
    float theta = max(0,dot(vNormal, vec3(0,1,0)));
    vec3 heightFogMod = vec3(1.0, 0.3, 1.0);
    vec3 diff = eyePosition - vPosition;
    float dist = length(diff*heightFogMod);
    float fog = clamp((fogEnd - dist) / (fogEnd - fogStart), 0.0, 1.0);
    color = vColor * min(1, theta + 0.01);
    color = color * fog + (1-fog) * vec3(0.0, 0.7, 1.0);
    depth = length(diff);
}
