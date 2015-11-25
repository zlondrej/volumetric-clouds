#version 430

in vec3 vColor;
in vec3 vNormal;

out vec3 color;

float fogEnd = 100;
float fogStart = 75;

void main() {
    float theta = max(0,dot(vNormal, vec3(0,1,0)));
    float fog = clamp((fogEnd - (gl_FragCoord.z/gl_FragCoord.w)) / (fogEnd - fogStart), 0.0, 1.0);
    color = vColor * min(1, theta + 0.01);
    color = color * fog + (1-fog) * vec3(0.0, 0.7, 1.0);
}
