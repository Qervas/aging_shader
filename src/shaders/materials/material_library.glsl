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

float createRipplePattern(vec3 pos) {
    float ripple = 0.0;
    // Create multiple ripple centers with more pronounced effect
    for (int i = 0; i < 5; i++) {
        vec2 center = vec2(sin(iTime * 0.5 + i), cos(iTime * 0.3 + i)) * 2.0;
        float dist = length(pos.xz - center);
        float wave = sin(dist * 8.0 - iTime * 3.0) * 0.8 + 0.2; // Increased amplitude
        ripple += wave * exp(-dist * 1.5); // Slower falloff
    }
    return ripple;
}

float getGroundHeight(vec2 pos) {
    // Create crater pattern
    float crater1 = exp(-length(pos + vec2(1.0, 0.5)) * 1.5);
    float crater2 = exp(-length(pos - vec2(2.0, -1.0)) * 2.0) * 0.7;
    float crater3 = exp(-length(pos - vec2(-1.5, 1.0)) * 1.0) * 0.5;

    // Add some noise for rough terrain
    float roughness = noise(vec3(pos * 2.0, 0.0)) * 0.2;

    // Combine craters and roughness
    return -(crater1 + crater2 + crater3) * 0.5 - roughness;
}

vec3 calculateGroundNormal(vec2 pos) {
    float eps = 0.01;
    float h = getGroundHeight(pos);
    float hx = getGroundHeight(pos + vec2(eps, 0.0));
    float hz = getGroundHeight(pos + vec2(0.0, eps));

    return normalize(vec3(
            (h - hx) / eps,
            1.0,
            (h - hz) / eps
        ));
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

vec3 getWetSurfaceColor(vec3 baseColor, vec3 normal, vec3 viewDir) {
    float wetness = moisture;
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 5.0);
    vec3 wetColor = mix(baseColor * 0.7, vec3(0.02), 0.5);
    return mix(baseColor, wetColor, wetness * (0.5 + 0.5 * fresnel));
}

Material createGroundMaterial(vec3 pos, vec3 normal, vec3 viewDir) {
    // Create base material
    Material mat = createBasicMaterial(
            vec3(0.2, 0.18, 0.15), // Slightly darker ground color
            0.0, // non-metallic base
            0.9, // rough by default
            1.5,
            normal
        );

    // Create crater rim effect
    float craterRim = smoothstep(0.3, 0.5, -getGroundHeight(pos.xz));

    // Add rust effect to crater rims
    float rustAmount = craterRim * rustLevel;
    vec4 rustPattern = getRustPattern(pos, rustAmount);
    vec3 rustColor = calculateRustColor(rustPattern);

    // Mix ground color with rust
    mat.albedo = mix(mat.albedo, rustColor, rustPattern.x);
    mat.metallic = mix(0.0, 0.3, rustPattern.x);
    mat.roughness = mix(mat.roughness, 0.7, rustPattern.x);

    float craterDepth = -getGroundHeight(pos.xz);
    float puddlePattern = smoothstep(0.1, 0.3, craterDepth) * moisture;

    if (puddlePattern > 0.01) {
        // More pronounced ripple effect
        float ripple = createRipplePattern(pos);
        mat.normal = normalize(normal + vec3(ripple * moisture * 0.3, 0.0, ripple * moisture * 0.3));

        // More reflective puddles
        vec3 puddleColor = vec3(0.02, 0.02, 0.03);
        float fresnel = pow(1.0 - max(dot(normal, -viewDir), 0.0), 3.0); // Increased fresnel effect
        mat.albedo = mix(mat.albedo, puddleColor, puddlePattern * (0.7 + 0.3 * fresnel));
        mat.roughness = mix(mat.roughness, 0.05, puddlePattern); // More reflective
        mat.metallic = mix(mat.metallic, 0.3, puddlePattern); // Slight metallic look for water
    }

    return mat;
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

vec3 getBrickColor(vec3 pos) {
    // Brick size and mortar thickness
    vec2 brickSize = vec2(0.4, 0.2);
    vec2 mortarThickness = vec2(0.02, 0.02);

    // Scale position for brick pattern
    vec3 scaledPos = pos * 2.0;

    // Offset alternate rows
    float rowOffset = floor(scaledPos.y / brickSize.y) * 0.5;
    scaledPos.x += rowOffset;

    // Calculate brick coordinates
    vec2 brickCoord = vec2(
            mod(scaledPos.x, brickSize.x),
            mod(scaledPos.y, brickSize.y)
        );

    // Determine if we're in mortar
    bool inMortar = brickCoord.x < mortarThickness.x ||
            brickCoord.y < mortarThickness.y ||
            brickCoord.x > (brickSize.x - mortarThickness.x) ||
            brickCoord.y > (brickSize.y - mortarThickness.y);

    // Base brick color variations
    vec3 brickBase = vec3(0.8, 0.3, 0.2);
    vec3 mortarColor = vec3(0.8, 0.8, 0.8);

    // Add some variation to brick color
    float variation = noise(pos * 10.0);
    brickBase *= 0.8 + variation * 0.4;

    // Return either brick or mortar color
    return inMortar ? mortarColor : brickBase;
}

Material createBrickMaterial(vec3 pos, vec3 normal) {
    vec3 baseColor = getBrickColor(pos);

    // Calculate bump mapping for mortar lines
    float eps = 0.01;
    vec3 dx = vec3(eps, 0.0, 0.0);
    vec3 dy = vec3(0.0, eps, 0.0);

    float bx = float(getBrickColor(pos + dx).r) - float(getBrickColor(pos - dx).r);
    float by = float(getBrickColor(pos + dy).r) - float(getBrickColor(pos - dy).r);

    // Create perturbed normal for bump mapping
    vec3 tangent = normalize(cross(normal, vec3(0.0, 1.0, 0.0)));
    vec3 bitangent = normalize(cross(normal, tangent));
    vec3 bumpedNormal = normalize(normal + (tangent * bx + bitangent * by) * 0.5);

    return createBasicMaterial(
        baseColor,
        0.0, // non-metallic
        0.95, // rough
        1.5, // IOR
        bumpedNormal
    );
}

#endif // MATERIAL_LIBRARY_GLSL
