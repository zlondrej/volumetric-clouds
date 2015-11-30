#version 430

uniform vec3 eyePosition;
uniform vec3 sunPosition = vec3(0.0, 1e10, 0.0);
uniform vec4 sunColor = vec4(1.0, 1.0, 1.0, 0.9); // a component is for intensity

in vec3 vPosition;
in vec3 vColor;
in vec3 vNormal;

layout(location = 0) out vec3 color;
layout(location = 1) out float depth;

float fogEnd = 100;
float fogStart = 75;

void main() {
    vec3 sunNormal = normalize(sunPosition - vPosition);
    float theta = max(0.01,dot(vNormal, sunNormal));
    vec3 heightFogMod = vec3(1.0, 0.3, 1.0);
    vec3 diff = eyePosition - vPosition;
    float dist = length(diff*heightFogMod);
    float fog = clamp((fogEnd - dist) / (fogEnd - fogStart), 0.0, 1.0);
    color = vColor * sunColor.rgb * sunColor.a * theta;
    color = color * fog + (1-fog) * vec3(0.0, 0.7, 1.0);
    depth = length(diff);
}
