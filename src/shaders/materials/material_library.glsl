#ifndef MATERIAL_LIBRARY_GLSL
#define MATERIAL_LIBRARY_GLSL

#include "../common/structures.glsl"
#include "../common/noise.glsl"
#include "../common/uniforms.glsl"

const vec3 STEEL_COLOR = vec3(0.8, 0.8, 0.8);
const vec3 RUST_COLOR = vec3(0.6, 0.2, 0.1);
const vec3 DARK_RUST = vec3(0.37, 0.15, 0.08); // Dark brown rust
const vec3 LIGHT_RUST = vec3(0.71, 0.29, 0.15); // Light orange rust
const vec3 RED_RUST = vec3(0.58, 0.21, 0.11); // Reddish rust
const vec3 BROWN_RUST = vec3(0.45, 0.22, 0.12); // Brown rust
const float RAIN_INTENSITY = 0.8;
const int NUM_RAIN_DROPS = 100;
const vec3 RAINY_SKY_COLOR = vec3(0.2, 0.2, 0.3);

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

vec3 getWetSurfaceColor(vec3 baseColor, vec3 normal, vec3 viewDir) {
    float wetness = moisture;
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 5.0);
    vec3 wetColor = mix(baseColor * 0.7, vec3(0.02), 0.5);
    return mix(baseColor, wetColor, wetness * (0.5 + 0.5 * fresnel));
}

Material createGroundMaterial(vec3 pos, vec3 normal, vec3 viewDir) {
    Material mat = createBasicMaterial(
            vec3(0.1), // darker for wet ground
            0.0,
            mix(0.9, 0.1, moisture), // more reflective when wet
            1.5,
            normal
        );

    // Add puddles
    float puddlePattern = noise(pos * 5.0);
    puddlePattern = smoothstep(0.3, 0.7, puddlePattern);

    // Make surface wet
    mat.albedo = getWetSurfaceColor(mat.albedo, normal, viewDir);
    mat.roughness = mix(mat.roughness, 0.1, puddlePattern * moisture);

    return mat;
}
float rainDrop(vec3 p) {
    float t = mod(iTime * 2.0, 10.0); // Rain animation time
    vec3 rp = p;
    rp.y += t * 3.0; // Rain falling speed
    float drop = noise(rp * 50.0);
    drop = smoothstep(0.95, 1.0, drop);
    return drop;
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
    float moistureEffect = noise(pos * 25.0) * moisture;

    // Enhance rust formation in areas with high moisture
    float moistureRust = smoothstep(0.4, 0.6, moistureEffect);
    rustPattern *= (1.0 + moistureRust * 0.5);

    // Make edges more susceptible to rust when moisture is high
    float edgeInfluence = pow(1.0 - abs(dot(normalize(pos), vec3(0.0, 1.0, 0.0))), 3.0);
    rustPattern = mix(rustPattern, rustPattern * (1.0 + edgeInfluence * moisture), 0.5);

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

float woodNoise(vec3 pos) {
    float grain = noise(pos * vec3(10.0, 1.0, 1.0));
    float rings = noise(pos * vec3(20.0, 2.0, 2.0));
    return mix(grain, rings, 0.5);
}

Material createWoodMaterial(vec3 pos, float age) {
    vec3 lightWood = vec3(0.7, 0.4, 0.2);
    vec3 darkWood = vec3(0.3, 0.2, 0.1);

    // Wood grain pattern
    float grain = woodNoise(pos);

    // Age effects
    float crack = noise(pos * 50.0 + age * 10.0);
    float weathering = noise(pos * 2.0 + age * 5.0);

    // Darken and add variation with age
    vec3 woodColor = mix(lightWood, darkWood, grain);
    woodColor = mix(woodColor, woodColor * 0.7, weathering * age);

    // Add cracks
    float crackPattern = smoothstep(0.7, 0.8, crack) * age;
    woodColor = mix(woodColor, darkWood * 0.5, crackPattern);

    // Calculate normal perturbation from cracks and grain
    vec3 normal = vec3(0.0, 0.0, 1.0);
    normal = normalize(normal + vec3(crack, crack, 0.0) * age * 0.1);

    return createBasicMaterial(
        woodColor,
        0.0, // non-metallic
        mix(0.5, 0.9, age), // rougher with age
        1.5, // IOR
        normal
    );
}

Material createPaintMaterial(vec2 uv, vec3 pos, float age) {
    // Sample base painting color
    vec3 paintColor = texture(paintingTexture, uv).rgb;

    // Cracking pattern
    float crackScale = 20.0 + age * 30.0;
    vec3 crackPos = pos * crackScale;
    float crack = noise(crackPos);
    float crackling = smoothstep(0.6, 0.7, crack) * age;

    // Peeling pattern
    float peelScale = 5.0 + age * 10.0;
    float peel = noise(pos * peelScale + age * 2.0);
    float peeling = smoothstep(0.7, 0.8, peel) * age;

    // Color aging
    vec3 agedColor = mix(paintColor, paintColor * 0.7, age * 0.5);

    // Add yellowing
    vec3 yellowTint = vec3(0.9, 0.8, 0.6);
    agedColor = mix(agedColor, agedColor * yellowTint, age * 0.3);

    // Apply cracking and peeling
    agedColor = mix(agedColor, vec3(0.2), crackling * 0.5);
    agedColor = mix(agedColor, vec3(0.1), peeling);

    // Normal perturbation from cracks and peeling
    vec3 normal = vec3(0.0, 0.0, 1.0);
    normal = normalize(normal +
                vec3(crack, crack, 0.0) * age * 0.2 +
                vec3(peel, peel, 0.0) * age * 0.3);

    return createBasicMaterial(
        agedColor,
        0.0, // non-metallic
        mix(0.2, 0.8, age), // rougher with age
        1.5, // IOR
        normal
    );
}

#endif // MATERIAL_LIBRARY_GLSL
