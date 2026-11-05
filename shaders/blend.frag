#version 430

#include "common.glsl"

uniform sampler2D sceneColor;  // linear HDR landscape color
uniform sampler2D sceneDepth;  // distance from eye, cleared to 1e15
uniform sampler2D cloudColor;  // low-res, rgb = scattered light, a = opacity
uniform sampler2D cloudDepth;  // low-res, scene distance used for each cloud texel

uniform vec3 eyePosition;
uniform mat4 invVP;

in vec2 textureCoords;

out vec4 fragColor;

vec3 acesFilm(vec3 x) {
    return clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
}

/**
 * Depth aware upsampling of the low resolution cloud buffer.
 *
 * Each low-res texel was ray-marched against the scene depth of one pixel.
 * Texels whose depth differs from this pixel's depth (e.g. terrain texels
 * for a sky pixel next to a mountain silhouette) are mostly ignored. A 3x3
 * neighbourhood is used so that a matching texel is almost always found.
 */
vec4 upsampleClouds(float depth) {
    ivec2 lowSize = textureSize(cloudColor, 0);
    vec2 lp = textureCoords * vec2(lowSize);
    ivec2 center = ivec2(floor(lp));

    vec4 acc = vec4(0.0);
    float wSum = 0.0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            ivec2 c = clamp(center + ivec2(x, y), ivec2(0), lowSize - 1);
            vec2 d = lp - (vec2(c) + 0.5);
            float spatial = exp(-2.0 * dot(d, d));

            float cd = texelFetch(cloudDepth, c, 0).r;
            float rel = abs(cd - depth) / max(min(cd, depth), 1.0);

            float w = spatial / (1e-3 + rel * rel);
            acc += texelFetch(cloudColor, c, 0) * w;
            wSum += w;
        }
    }
    return acc / max(wSum, 1e-6);
}

/** Cheap cloud shadow: sample cloud density where the sun ray crosses the cloud layer. */
float cloudShadow(vec3 p) {
    float sy = max(sunDirection.y, 0.1);
    float od = 0.0;
    for (int i = 0; i < 3; i++) {
        float y = cloudBottom + cloudThickness * (0.15 + 0.25 * float(i));
        // Only clouds above the point (mountain tops can be in or above the layer).
        if (y > p.y) {
            vec3 sp = p + sunDirection * ((y - p.y) / sy);
            od += cloudDensity(sp, 24.0, false);
        }
    }
    return exp(-od * 2.5);
}

void main() {
    ivec2 px = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(sceneDepth, px, 0).r;

    vec4 nearPoint = invVP * vec4(textureCoords * 2.0 - 1.0, -1.0, 1.0);
    vec3 rd = normalize(nearPoint.xyz / nearPoint.w);

    vec3 col;
    if (depth > 1e10) {
        col = skyColor(rd);
        float s = dot(rd, sunDirection);
        col += sunColor * 6.0 * smoothstep(0.9997, 0.99985, s);
    } else {
        vec3 p = eyePosition + rd * depth;
        vec4 scene = texelFetch(sceneColor, px, 0);
        // Clouds block only the direct sunlight (scene.a is its share of the
        // pixel's light), sky light stays.
        float sunFraction = scene.a;
        col = scene.rgb * (1.0 - sunFraction * (1.0 - cloudShadow(p)));
        // Same aerial perspective as the clouds.
        float fog = 1.0 - exp(-depth / hazeDistance);
        col = mix(col, skyColor(rd), fog);
    }

    vec4 clouds = upsampleClouds(depth);
    col = col * (1.0 - clouds.a) + clouds.rgb;

    col = acesFilm(col * 0.9);
    col = pow(col, vec3(1.0 / 2.2));

    // Dither to avoid banding in the sky gradient.
    float dither = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
    col += (dither - 0.5) / 255.0;

    fragColor = vec4(col, 1.0);
}
