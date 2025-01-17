#ifndef STRUCTURES_GLSL
#define STRUCTURES_GLSL

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ior;
    vec3 normal;
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

struct Rectangle {
    vec3 center;
    vec3 normal;
    vec3 up;
    float width;
    float height;
    Material material;
};

struct Frame {
    Rectangle painting;
    float frameWidth;
    Material frameMaterial;
};

#endif // STRUCTURES_GLSL
