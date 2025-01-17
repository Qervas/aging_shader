#ifndef MATERIAL_LIBRARY_GLSL
#define MATERIAL_LIBRARY_GLSL

#include "../common/structures.glsl"
#include "../common/noise.glsl"

const vec3 STEEL_COLOR = vec3(0.8, 0.8, 0.8);
const vec3 RUST_COLOR = vec3(0.6, 0.2, 0.1);
const vec3 DARK_RUST = vec3(0.37, 0.15, 0.08); // Dark brown rust
const vec3 LIGHT_RUST = vec3(0.71, 0.29, 0.15); // Light orange rust
const vec3 RED_RUST = vec3(0.58, 0.21, 0.11); // Reddish rust
const vec3 BROWN_RUST = vec3(0.45, 0.22, 0.12); // Brown rust

Material createBasicMaterial(vec3 albedo, float metallic, float roughness, float ior, vec3 normal) {
    Material mat;
    mat.albedo = albedo;
    mat.metallic = metallic;
    mat.roughness = roughness;
    mat.ior = ior;
    mat.normal = normal;
    return mat;
}

Material createGoldMaterial() {
    return createBasicMaterial(
        vec3(1.0, 0.765, 0.336), // albedo
        1.0, // metallic
        0.1, // roughness
        0.47, // IOR
        vec3(0.0, 1.0, 0.0) // default normal
    );
}

Material createGroundMaterial() {
    return createBasicMaterial(
        vec3(0.2, 0.2, 0.2), // albedo
        0.0, // metallic
        0.8, // roughness
        1.5, // IOR
        vec3(0.0, 1.0, 0.0) // default normal
    );
}
vec4 getRustPattern(vec3 pos, float rustLevel) {
    // Basic noise scales
    const float largeScale = 2.0;
    const float mediumScale = 5.0;
    const float smallScale = 15.0;
    const float microScale = 30.0;

    // Base rust pattern (large areas)
    float baseRust = noise(pos * largeScale);

    // Medium detail
    float mediumDetail = noise(pos * mediumScale + vec3(baseRust));

    // Fine detail (cracks and spots)
    float fineDetail = noise(pos * smallScale + vec3(mediumDetail));

    // Micro detail (surface roughness)
    float microDetail = noise(pos * microScale);

    // Combine patterns with different weights
    float rustPattern = baseRust * 0.5 +
            mediumDetail * 0.3 +
            fineDetail * 0.15 +
            microDetail * 0.05;

    // Create edge weathering effect
    float edgeRust = pow(1.0 - abs(dot(normalize(pos), vec3(0.0, 1.0, 0.0))), 3.0);
    rustPattern = mix(rustPattern, rustPattern * (1.0 + edgeRust), 0.5);

    // Apply global rust level
    rustPattern *= rustLevel;

    // Create different rust layers
    float deepRust = smoothstep(0.3, 0.7, rustPattern);
    float surfaceRust = smoothstep(0.1, 0.4, rustPattern);
    float lightRust = smoothstep(0.0, 0.3, rustPattern);

    // Calculate surface displacement for normal mapping
    float displacement = mix(0.0, 0.02, rustPattern);

    return vec4(rustPattern, deepRust, surfaceRust, displacement);
}

vec3 calculateRustColor(vec4 rustPattern) {
    float deep = rustPattern.y;
    float surface = rustPattern.z;

    // Mix different rust colors based on depth
    vec3 rustColor = mix(STEEL_COLOR, LIGHT_RUST, surface);
    rustColor = mix(rustColor, RED_RUST, deep * 0.7);
    rustColor = mix(rustColor, DARK_RUST, deep * deep * 0.5);

    // Add slight color variation
    float variation = noise(vec3(rustPattern.x * 42.0));
    rustColor *= 0.8 + variation * 0.4;

    return rustColor;
}

vec3 calculateRustNormal(vec3 pos, vec4 rustPattern, vec3 normal) {
    // Calculate normal perturbation based on rust pattern
    float eps = 0.01;
    vec3 dx = vec3(eps, 0.0, 0.0);
    vec3 dy = vec3(0.0, eps, 0.0);
    vec3 dz = vec3(0.0, 0.0, eps);

    float rx = getRustPattern(pos + dx, 1.0).w - getRustPattern(pos - dx, 1.0).w;
    float ry = getRustPattern(pos + dy, 1.0).w - getRustPattern(pos - dy, 1.0).w;
    float rz = getRustPattern(pos + dz, 1.0).w - getRustPattern(pos - dz, 1.0).w;

    vec3 tangent = normalize(cross(normal, vec3(0.0, 1.0, 0.0)));
    vec3 bitangent = normalize(cross(normal, tangent));

    // Construct perturbed normal
    vec3 bumpNormal = normalize(normal + (tangent * rx + bitangent * ry + normal * rz) * rustPattern.w * 10.0);
    return bumpNormal;
}

Material createSteelMaterial(float rustLevel, vec3 worldPos, vec3 normal) {
    // Get rust pattern and displacement
    vec4 rustPattern = getRustPattern(worldPos, rustLevel);

    // Calculate final color
    vec3 baseColor = calculateRustColor(rustPattern);

    // Calculate perturbed normal
    vec3 bumpedNormal = calculateRustNormal(worldPos, rustPattern, normal);

    // Adjust material properties based on rust
    float pattern = rustPattern.x;
    float metallic = mix(1.0, 0.0, pattern * 0.9); // Rust is less metallic
    float roughness = mix(0.3, 0.95, pattern); // Rust is rougher

    Material mat;
    mat.albedo = baseColor;
    mat.metallic = metallic;
    mat.roughness = roughness;
    mat.ior = 2.5;
    mat.normal = bumpedNormal;
    return mat;
}

#endif // MATERIAL_LIBRARY_GLSL
