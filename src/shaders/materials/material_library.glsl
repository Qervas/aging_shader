#ifndef MATERIAL_LIBRARY_GLSL
#define MATERIAL_LIBRARY_GLSL

#include "../common/structures.glsl"
#include "../common/noise.glsl"

const vec3 STEEL_COLOR = vec3(0.8, 0.8, 0.8);
const vec3 RUST_COLOR = vec3(0.6, 0.2, 0.1);

Material createGoldMaterial() {
    return Material(
        vec3(1.0, 0.765, 0.336), // albedo
        1.0, // metallic
        0.1, // roughness
        0.47 // IOR
    );
}

Material createGroundMaterial() {
    return Material(
        vec3(0.2, 0.2, 0.2), // albedo
        0.0, // metallic
        0.8, // roughness
        1.5 // IOR
    );
}

Material createSteelMaterial(float rustLevel, vec3 worldPos) {
    // Transform world position to local coordinates for stable noise pattern
    vec3 localPos = worldPos;

    // Reduce the noise scale to make the pattern more visible
    float noiseScale = 2.0;

    // Use multiple noise octaves for more interesting patterns
    float noiseValue = 0.0;
    float amplitude = 1.0;
    float frequency = 1.0;

    for (int i = 0; i < 3; i++) {
        noiseValue += amplitude * noise(localPos * noiseScale * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }

    // Normalize noise value
    noiseValue = noiseValue / 1.75;

    // Create patchy rust effect
    float localRust = mix(
            rustLevel - 0.3,
            rustLevel + 0.3,
            noiseValue
        );
    localRust = clamp(localRust, 0.0, 1.0);

    // Interpolate between steel and rust properties
    vec3 albedo = mix(STEEL_COLOR, RUST_COLOR, localRust);
    float metallic = mix(1.0, 0.3, localRust);
    float roughness = mix(0.1, 0.8, localRust);

    return Material(
        albedo,
        metallic,
        roughness,
        2.5
    );
}

#endif // MATERIAL_LIBRARY_GLSL
