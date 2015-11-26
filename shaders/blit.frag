#version 430

uniform sampler2D texture;

in vec2 textureCoords;

out vec3 color;

void main() {
  color = texture2D(texture, textureCoords).rgb;
}
