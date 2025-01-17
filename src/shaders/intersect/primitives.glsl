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
    // Use ray marching for the uneven ground
    float t = 0.0;
    float maxDist = 100.0;
    float minDist = 0.001;
    int maxSteps = 64;

    for (int i = 0; i < maxSteps; i++) {
        vec3 p = ray.origin + ray.direction * t;
        float h = getGroundHeight(p.xz);
        float d = p.y - (GROUND_Y + h);

        if (d < minDist) {
            hitInfo.hit = true;
            hitInfo.t = t;
            hitInfo.position = p;
            hitInfo.normal = calculateGroundNormal(p.xz);
            hitInfo.material = createGroundMaterial(
                    hitInfo.position,
                    hitInfo.normal,
                    -ray.direction
                );
            return true;
        }

        if (t > maxDist) break;
        t += max(d * 0.5, minDist);
    }

    return false;
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

bool intersectWall(Ray ray, vec3 position, vec3 normal, float width, float height, float thickness, out HitInfo hitInfo) {
    float denom = dot(ray.direction, normal);

    if (abs(denom) > 0.0001) {
        vec3 po = position - ray.origin;
        float t = dot(po, normal) / denom;

        if (t > 0.0) {
            vec3 p = ray.origin + t * ray.direction - position;
            vec3 up = vec3(0.0, 1.0, 0.0);
            vec3 right = normalize(cross(up, normal));

            float x = dot(p, right);
            float y = dot(p, up);

            if (abs(x) < width * 0.5 && y > 0.0 && y < height) {
                hitInfo.hit = true;
                hitInfo.t = t;
                hitInfo.position = ray.origin + t * ray.direction;
                hitInfo.normal = normal;
                hitInfo.material = createBrickMaterial(hitInfo.position, normal);
                return true;
            }
        }
    }
    return false;
}

#endif // PRIMITIVES_GLSL
