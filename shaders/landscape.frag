#version 430

#include "terrain.glsl"

uniform vec3 eyePosition;
uniform vec3 sunDirection;
uniform vec4 discardRect;  // xz area covered by the finer grid level (min.xy, max.zw)

in vec3 vPosition;
in vec3 vNormal;

// rgb = lit color, a = fraction of the light that comes directly from the sun
// (used to apply cloud shadows in the composite pass)
layout(location = 0) out vec4 color;
layout(location = 1) out float depth;

void main() {
    if (all(greaterThan(vPosition.xz, discardRect.xy)) && all(lessThan(vPosition.xz, discardRect.zw))) {
        discard;
    }

    vec3 n = normalize(vNormal);
    float y = vPosition.y;

    // Albedos in linear space.
    float variation = fbm(vPosition.xz / 120.0, 3);
    vec3 grass = mix(vec3(0.035, 0.075, 0.015), vec3(0.09, 0.10, 0.03), smoothstep(0.3, 0.7, variation));
    vec3 rock = mix(vec3(0.14, 0.12, 0.10), vec3(0.22, 0.20, 0.18), variation);
    vec3 snow = vec3(0.85, 0.88, 0.92);

    float grassAmount = smoothstep(0.75, 0.9, n.y) * (1.0 - smoothstep(80.0, 115.0, y + variation * 25.0));
    vec3 albedo = mix(rock, grass, grassAmount);

    float snowAmount = smoothstep(125.0, 140.0, y + variation * 20.0) * smoothstep(0.45, 0.65, n.y);
    albedo = mix(albedo, snow, snowAmount);

    float diffuse = max(dot(n, sunDirection), 0.0);
    vec3 sun = vec3(1.0, 0.94, 0.86) * 3.2 * diffuse;
    vec3 sky = vec3(0.55, 0.65, 0.85) * (0.6 + 0.4 * n.y);
    // Some light bounced from the ground.
    sky += vec3(0.25, 0.25, 0.2) * (0.5 - 0.5 * n.y);

    const vec3 luminance = vec3(0.2126, 0.7152, 0.0722);
    float sunFraction = dot(sun, luminance) / max(dot(sun + sky, luminance), 1e-4);

    color = vec4(albedo * (sun + sky), sunFraction);
    depth = length(eyePosition - vPosition);
}
