#version 430

uniform mat4 view;
uniform mat4 projection;

in vec3 position;
in vec3 color;

out vec3 vColor;

void main() {
  vColor = color;

  mat4 mvp = projection * view;

  gl_Position = mvp * vec4(position, 1);
}
