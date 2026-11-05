// Mountain positions shared by the terrain and the clouds.

// xy = position, z = height, w = radius
const int mountainCount = 5;
const vec4 mountains[mountainCount] = vec4[](
    vec4(  650.0,  1500.0, 110.0,  700.0),
    vec4(-1500.0,  1100.0,  95.0,  600.0),
    vec4( 2300.0,  -300.0, 120.0,  800.0),
    vec4( -700.0, -2100.0, 105.0,  700.0),
    vec4(-2600.0, -1200.0,  90.0,  600.0)
);

/** 1 over a mountain, fading to 0 a bit beyond its foothills. */
float mountainInfluence(vec2 p) {
    float influence = 0.0;
    for (int i = 0; i < mountainCount; i++) {
        float r = length(p - mountains[i].xy) / mountains[i].w;
        influence = max(influence, 1.0 - smoothstep(0.4, 1.5, r));
    }
    return influence;
}
