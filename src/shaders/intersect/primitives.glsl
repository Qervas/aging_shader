#ifndef PRIMITIVES_GLSL
#define PRIMITIVES_GLSL

#include "../common/structures.glsl"
#include "../common/constants.glsl"

bool intersectSphere(Ray ray, Sphere sphere, out HitInfo hitInfo) {
    vec3 oc = ray.origin - sphere.center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) return false;

    float t = (-b - sqrt(discriminant)) / (2.0 * a);
    if (t < 0.0) return false;

    hitInfo.hit = true;
    hitInfo.t = t;
    hitInfo.position = ray.origin + t * ray.direction;
    hitInfo.normal = normalize(hitInfo.position - sphere.center);
    hitInfo.material = sphere.material;
    return true;
}

bool intersectGround(Ray ray, out HitInfo hitInfo) {
    if (ray.direction.y >= 0.0) return false;

    float t = -(ray.origin.y - GROUND_Y) / ray.direction.y;
    if (t < 0.0) return false;

    hitInfo.hit = true;
    hitInfo.t = t;
    hitInfo.position = ray.origin + t * ray.direction;
    hitInfo.normal = vec3(0.0, 1.0, 0.0);
    hitInfo.material = Material(
            vec3(0.2, 0.2, 0.2), // albedo
            0.0, // metallic
            0.8, // roughness
            1.5 // IOR
        );

    float scale = 2.0;
    bool x = mod(floor(hitInfo.position.x * scale), 2.0) == 0.0;
    bool z = mod(floor(hitInfo.position.z * scale), 2.0) == 0.0;
    if (x ^^ z) {
        hitInfo.material.albedo *= 0.5;
    }

    return true;
}

#endif // PRIMITIVES_GLSL
