#ifndef BRDF_GLSL
#define BRDF_GLSL

#include "../common/constants.glsl"

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 calculatePBR(HitInfo hit, vec3 viewDir) {
    // Use the potentially perturbed normal from the material
    vec3 N = hit.normal;
    vec3 V = -viewDir;
    vec3 L = LIGHT_DIR;
    vec3 H = normalize(V + L);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, hit.material.albedo, hit.material.metallic);

    // Add cavity shadowing for deep rust
    float cavityAO = 1.0 - (1.0 - hit.material.metallic) * 0.5;

    float NDF = DistributionGGX(N, H, hit.material.roughness);
    float G = GeometrySmith(N, V, L, hit.material.roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - hit.material.metallic;

    float NdotL = max(dot(N, L), 0.0);
    vec3 color = (kD * hit.material.albedo / PI + specular) *
            LIGHT_COLOR * LIGHT_INTENSITY * NdotL * cavityAO;

    // Add ambient occlusion based on rust level
    vec3 ambient = vec3(0.03) * hit.material.albedo * cavityAO;

    return color + ambient;
}

#endif // BRDF_GLSL
