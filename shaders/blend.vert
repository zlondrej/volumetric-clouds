#version 430

// Full screen triangle, no vertex buffers needed.

out vec2 textureCoords;

void main() {
    vec2 position = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2) * 2.0 - 1.0;
    gl_Position = vec4(position, 0.0, 1.0);
    textureCoords = (position + 1.0) / 2.0;
}
