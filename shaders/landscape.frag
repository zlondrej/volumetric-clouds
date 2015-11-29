#version 430

uniform vec3 eyePosition;

in vec3 vPosition;
in vec3 vColor;
in vec3 vNormal;

out vec3 color;

float fogEnd = 100;
float fogStart = 75;

void main() {
    float theta = max(0,dot(vNormal, vec3(0,1,0)));
    vec3 heightFogMod = vec3(1.0, 0.3, 1.0);
    float dist = distance(eyePosition*heightFogMod, vPosition*heightFogMod);
    float fog = clamp((fogEnd - dist) / (fogEnd - fogStart), 0.0, 1.0);
    color = vColor * min(1, theta + 0.01);
    color = color * fog + (1-fog) * vec3(0.0, 0.7, 1.0);
}
