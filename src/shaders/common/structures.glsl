#ifndef STRUCTURES_GLSL
#define STRUCTURES_GLSL

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ior;
};

struct Sphere {
    vec3 center;
    float radius;
    Material material;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct HitInfo {
    bool hit;
    float t;
    vec3 position;
    vec3 normal;
    Material material;
};

#endif // STRUCTURES_GLSL
