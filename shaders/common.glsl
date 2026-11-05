// Shared code for cloud rendering (included by clouds.comp and blend.frag).

#define PI 3.141592653589793

#include "mountains.glsl"

uniform sampler3D noiseTex;   // R = Perlin-Worley, GBA = Worley FBM at 3 frequencies
uniform sampler2D weatherTex; // large scale coverage
uniform vec3 sunDirection;    // normalized, pointing towards the sun
uniform float time;           // seconds
uniform float coverage;       // 0..1

const float cloudBottom = 90.0;
const float cloudTop = 210.0;
const float cloudThickness = cloudTop - cloudBottom;

// Larger than the view distance, so that the repetition isn't visible.
const float weatherScale = 20000.0; // world units per weather tile
const float shapeScale = 520.0;     // world units per base shape tile
const float detailScale = 96.0;     // world units per detail tile
const float noiseRes = 128.0;       // 3D noise texture resolution
const float weatherRes = 512.0;     // weather texture resolution

const vec3 windDirection = vec3(0.8, 0.0, 0.6);
const float windSpeed = 6.0;

const vec3 sunColor = vec3(1.0, 0.94, 0.86) * 2.6;

// Distance over which the atmosphere fades things into the sky color (1/e).
const float hazeDistance = 4000.0;

float saturate(float x) {
    return clamp(x, 0.0, 1.0);
}

float remap(float v, float lo, float hi, float nlo, float nhi) {
    return nlo + (v - lo) / (hi - lo) * (nhi - nlo);
}

vec3 skyColor(vec3 rd) {
    float y = max(rd.y, 0.0);
    vec3 zenith = vec3(0.10, 0.28, 0.70);
    vec3 horizon = vec3(0.62, 0.74, 0.90);
    vec3 col = mix(horizon, zenith, sqrt(y));
    // Darker haze below the horizon.
    col = mix(col, vec3(0.36, 0.42, 0.50), 1.0 - exp(min(rd.y, 0.0) * 6.0));

    float s = max(dot(rd, sunDirection), 0.0);
    col += vec3(1.0, 0.75, 0.5) * (pow(s, 6.0) * 0.18 + pow(s, 48.0) * 0.35);
    return col;
}

vec3 ambientColor(float heightFraction) {
    return mix(vec3(0.30, 0.36, 0.45), vec3(0.62, 0.72, 0.90), heightFraction);
}

/**
 * Texture LOD for a sample of given footprint (world units) on a texture of
 * given resolution whose tile covers `scale` world units.
 */
float textureLodFor(float footprint, float scale, float resolution) {
    return max(0.0, log2(footprint * resolution / scale));
}

float noiseLod(float footprint, float scale) {
    return textureLodFor(footprint, scale, noiseRes);
}

vec3 windOffset() {
    return windDirection * windSpeed * time;
}

/** Large scale coverage, 0..1 */
float weather(vec2 xz, float lod) {
    vec2 uv = (xz + windOffset().xz * 0.5) / weatherScale;
    float w = textureLod(weatherTex, uv, lod).r;
    // Mountains force air upwards, so they are cloudier (orographic clouds).
    // This uses the world position, not the wind offset: the cloudiness stays
    // over the mountains while the clouds themselves move through.
    float mountain = mountainInfluence(xz);
    // Slightly more clouds where there would be some anyway...
    float threshold = 0.58 - coverage * 0.3 - mountain * 0.06;
    float cover = saturate(remap(w, threshold, threshold + 0.2, 0.0, 1.0));
    // ...and at least scattered broken clouds even when it's clear elsewhere.
    float minimum = mountain * 0.55;
    return max(cover, minimum * mix(0.6, 1.0, w));
}

/** Vertical profile of a cloud column whose weather value is w. */
float heightGradient(float h, float w) {
    float top = 0.35 + 0.65 * w;
    return smoothstep(0.0, 0.08, h) * (1.0 - smoothstep(top * 0.4, top, h));
}

/**
 * Cloud density at point p.
 * footprint - approximate size of the sample in world units (for LOD selection)
 * detail - whether to apply high frequency erosion
 */
float cloudDensity(vec3 p, float footprint, bool detail) {
    float h = (p.y - cloudBottom) / cloudThickness;
    if (h <= 0.0 || h >= 1.0) {
        return 0.0;
    }

    float w = weather(p.xz, textureLodFor(footprint, weatherScale, weatherRes));
    float hg = heightGradient(h, w);

    // Cheap rejection: base shape is at most 1, so this column can't contain cloud.
    if (hg * w < 0.02) {
        return 0.0;
    }

    vec3 wind = windOffset();
    vec3 sp = (p + wind + vec3(0.0, time * 0.5, 0.0)) / shapeScale;
    vec4 n = textureLod(noiseTex, sp, noiseLod(footprint, shapeScale));
    float fbm = n.g * 0.625 + n.b * 0.25 + n.a * 0.125;
    float base = saturate(remap(n.r, fbm - 1.0, 1.0, 0.0, 1.0));

    float d = saturate(remap(base * hg, 1.0 - w, 1.0, 0.0, 1.0));

    if (detail && d > 0.0) {
        vec3 dp = (p + wind * 1.6 + vec3(0.0, -time * 1.5, 0.0)) / detailScale;
        vec3 dn = textureLod(noiseTex, dp, noiseLod(footprint, detailScale)).gba;
        float dfbm = dn.x * 0.625 + dn.y * 0.25 + dn.z * 0.125;
        // Wispy at the bottom, billowy at the top.
        float erosion = mix(1.0 - dfbm, dfbm, saturate(h * 4.0)) * 0.55;
        d = saturate(remap(d, erosion, 1.0, 0.0, 1.0));
    }

    return d;
}
