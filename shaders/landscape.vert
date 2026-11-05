#version 430

#include "terrain.glsl"

// One level of a set of nested grids (a simple geometry clipmap). The grid
// has no vertex buffer, positions come from gl_VertexID.

uniform mat4 view;
uniform mat4 projection;

uniform vec2 levelOrigin;  // world xz of the grid center, snapped to 2 * spacing
uniform float spacing;     // distance between vertices
uniform int gridSize;      // number of quads along one side

out vec3 vPosition;
out vec3 vNormal;

void main() {
    int rowLength = gridSize + 1;
    ivec2 g = ivec2(gl_VertexID % rowLength, gl_VertexID / rowLength) - gridSize / 2;

    vec2 xz = levelOrigin + vec2(g) * spacing;
    float h = terrainHeight(xz);

    float hx = terrainHeight(xz + vec2(spacing, 0.0));
    float hz = terrainHeight(xz + vec2(0.0, spacing));

    vPosition = vec3(xz.x, h, xz.y);
    vNormal = normalize(vec3(h - hx, spacing, h - hz));

    gl_Position = projection * view * vec4(vPosition, 1.0);
}
