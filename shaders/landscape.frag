#version 430

in vec3 vColor;
in vec3 vNormal;

out vec3 color;

void main() {
    float theta = max(0,dot(vNormal, vec3(0,1,0)));
    color = vColor * min(1, theta + 0.01);
}
