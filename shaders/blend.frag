#version 430

uniform sampler2D frontTexture;
uniform sampler2D backTexture;
uniform sampler2D frontDepth;
uniform sampler2D backDepth;

in vec2 textureCoords;

out vec3 color;

void main() {
  vec4 front = texture(frontTexture, textureCoords);
  vec4 back = texture(backTexture, textureCoords);

  vec4 frontD = texture(frontDepth, textureCoords);
  vec4 backD = texture(backDepth, textureCoords);

  if(backD.x <= frontD.x) {
    color = back.rgb;
  } else {
    color = mix(back.rgb, front.rgb, front.a);
  }
}
