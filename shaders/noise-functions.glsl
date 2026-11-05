// Tileable noise functions used to generate noise textures.
// All functions take coordinates in [0, 1) and a frequency (cells per tile).

vec3 hash33(vec3 p) {
    uvec3 q = uvec3(ivec3(p) + 1024) * uvec3(1597334673u, 3812015801u, 2798796415u);
    q = (q.x ^ q.y ^ q.z) * uvec3(1597334673u, 3812015801u, 2798796415u);
    return vec3(q) * (1.0 / 4294967295.0);
}

float worley(vec3 uvw, float freq) {
    vec3 p = uvw * freq;
    vec3 id = floor(p);
    vec3 f = p - id;
    float minDist = 1.0;

    for (int z = -1; z <= 1; z++)
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++) {
        vec3 offset = vec3(x, y, z);
        vec3 h = hash33(mod(id + offset, freq));
        vec3 d = offset + h - f;
        minDist = min(minDist, dot(d, d));
    }

    return 1.0 - sqrt(minDist);
}

float worleyFbm(vec3 uvw, float freq) {
    return worley(uvw, freq) * 0.625
         + worley(uvw, freq * 2.0) * 0.25
         + worley(uvw, freq * 4.0) * 0.125;
}

vec3 gradient(vec3 cell, float freq) {
    return normalize(hash33(mod(cell, freq)) * 2.0 - 1.0);
}

float perlin(vec3 uvw, float freq) {
    vec3 p = uvw * freq;
    vec3 i = floor(p);
    vec3 f = p - i;
    vec3 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

    float n000 = dot(gradient(i + vec3(0, 0, 0), freq), f - vec3(0, 0, 0));
    float n100 = dot(gradient(i + vec3(1, 0, 0), freq), f - vec3(1, 0, 0));
    float n010 = dot(gradient(i + vec3(0, 1, 0), freq), f - vec3(0, 1, 0));
    float n110 = dot(gradient(i + vec3(1, 1, 0), freq), f - vec3(1, 1, 0));
    float n001 = dot(gradient(i + vec3(0, 0, 1), freq), f - vec3(0, 0, 1));
    float n101 = dot(gradient(i + vec3(1, 0, 1), freq), f - vec3(1, 0, 1));
    float n011 = dot(gradient(i + vec3(0, 1, 1), freq), f - vec3(0, 1, 1));
    float n111 = dot(gradient(i + vec3(1, 1, 1), freq), f - vec3(1, 1, 1));

    float nx00 = mix(n000, n100, u.x);
    float nx10 = mix(n010, n110, u.x);
    float nx01 = mix(n001, n101, u.x);
    float nx11 = mix(n011, n111, u.x);

    return mix(mix(nx00, nx10, u.y), mix(nx01, nx11, u.y), u.z);
}

/** Fractal Perlin noise remapped to roughly [0, 1]. */
float perlinFbm(vec3 uvw, float freq, int octaves) {
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < octaves; i++) {
        v += perlin(uvw, freq) * a;
        freq *= 2.0;
        a *= 0.5;
    }
    return clamp(v * 0.9 + 0.5, 0.0, 1.0);
}
