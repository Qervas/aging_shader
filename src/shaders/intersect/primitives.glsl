#ifndef PRIMITIVES_GLSL
#define PRIMITIVES_GLSL

#include "../common/structures.glsl"
#include "../common/constants.glsl"
#include "../common/uniforms.glsl"

bool intersectSphere(Ray ray, Sphere sphere, float rustLevel, out HitInfo hitInfo) {
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
    hitInfo.material = createSteelMaterial(rustLevel,
            hitInfo.position - sphere.center,
            hitInfo.normal);
    // Use the perturbed normal from the material
    hitInfo.normal = hitInfo.material.normal;
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
    hitInfo.material = createBasicMaterial(
            vec3(0.2, 0.2, 0.2), // albedo
            0.0, // metallic
            0.8, // roughness
            1.5, // IOR
            vec3(0.0, 1.0, 0.0) // normal
        );

    float scale = 2.0;
    bool x = mod(floor(hitInfo.position.x * scale), 2.0) == 0.0;
    bool z = mod(floor(hitInfo.position.z * scale), 2.0) == 0.0;
    if (x ^^ z) {
        hitInfo.material.albedo *= 0.5;
    }

    return true;
}

bool intersectRectangle(Ray ray, Rectangle rect, out HitInfo hitInfo) {
    float denom = dot(ray.direction, rect.normal);

    if (abs(denom) > 0.0001) {
        vec3 po = rect.center - ray.origin;
        float t = dot(po, rect.normal) / denom;

        if (t > 0.0) {
            vec3 p = ray.origin + t * ray.direction - rect.center;
            vec3 right = normalize(cross(rect.up, rect.normal));

            float x = dot(p, right);
            float y = dot(p, rect.up);

            if (abs(x) < rect.width * 0.5 && abs(y) < rect.height * 0.5) {
                hitInfo.hit = true;
                hitInfo.t = t;
                hitInfo.position = ray.origin + t * ray.direction;
                hitInfo.normal = rect.normal;

                // Calculate UV coordinates
                vec2 uv = vec2(
                        (x + rect.width * 0.5) / rect.width,
                        (y + rect.height * 0.5) / rect.height
                    );

                // Create material based on position
                if (abs(x) > (rect.width * 0.5 - frameWidth) ||
                        abs(y) > (rect.height * 0.5 - frameWidth)) {
                    // Frame material
                    hitInfo.material = createWoodMaterial(hitInfo.position, age);
                } else {
                    // Painting material
                    hitInfo.material = createPaintMaterial(uv, hitInfo.position, age);
                }

                return true;
            }
        }
    }

    return false;
}

#endif // PRIMITIVES_GLSL
