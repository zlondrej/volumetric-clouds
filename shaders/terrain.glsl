// Procedural terrain height field.

float terrainHash(ivec2 p) {
    uvec2 q = uvec2(p) * uvec2(1597334673u, 3812015801u);
    uint n = (q.x ^ q.y) * 1597334673u;
    return float(n) * (1.0 / 4294967295.0);
}

float valueNoise(vec2 p) {
    vec2 fl = floor(p);
    ivec2 i = ivec2(fl);
    vec2 f = p - fl;
    vec2 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

    float a = terrainHash(i);
    float b = terrainHash(i + ivec2(1, 0));
    float c = terrainHash(i + ivec2(0, 1));
    float d = terrainHash(i + ivec2(1, 1));

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

const mat2 octaveRotation = mat2(0.8, -0.6, 0.6, 0.8);

float fbm(vec2 p, int octaves) {
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < octaves; i++) {
        v += a * valueNoise(p);
        p = octaveRotation * p * 2.0 + 17.3;
        a *= 0.5;
    }
    return v;
}

float ridgedFbm(vec2 p, int octaves) {
    float v = 0.0;
    float a = 0.5;
    float weight = 1.0;
    for (int i = 0; i < octaves; i++) {
        float n = 1.0 - abs(valueNoise(p) * 2.0 - 1.0);
        n *= n * weight;
        // Finer ridges mostly appear on top of the larger ones.
        weight = clamp(n * 2.0, 0.0, 1.0);
        v += a * n;
        p = octaveRotation * p * 2.0 + 31.7;
        a *= 0.5;
    }
    return v;
}

#include "mountains.glsl"

float terrainHeight(vec2 p) {
    // Rolling hills
    float h = fbm(p / 260.0, 4) * 75.0 - 10.0;

    // Warp the mountain footprints so they aren't circular.
    vec2 warp = vec2(valueNoise(p / 500.0), valueNoise(p / 500.0 + 7.1)) * 2.0 - 1.0;

    for (int i = 0; i < mountainCount; i++) {
        vec4 m = mountains[i];
        vec2 d = (p - m.xy + warp * m.w * 0.4) / m.w;
        float r2 = dot(d, d);
        if (r2 < 1.0) {
            // Peaked profile with soft foothills.
            float r = sqrt(r2);
            float falloff = (1.0 - r) * (1.0 - r) * (1.0 + 2.0 * r);
            float ridges = ridgedFbm(p / 200.0 + float(i) * 13.1, 6);
            h += m.z * falloff * (0.3 + 1.0 * ridges);
        }
    }

    return h;
}
