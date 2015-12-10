#version 430

uniform sampler2D frontTexture;
uniform sampler2D backTexture;
uniform sampler2D frontDepth;
uniform sampler2D backDepth;

in vec2 textureCoords;

out vec3 color;

void main() {
  vec4 front = texture2D(frontTexture, textureCoords);
  vec4 back = texture2D(backTexture, textureCoords);

  vec4 frontD = texture2D(frontDepth, textureCoords);
  vec4 backD = texture2D(backDepth, textureCoords);

  if(backD.x <= frontD.x) {
    color = back.rgb;
  } else {
    color = mix(back.rgb, front.rgb, front.a);
  }
}
