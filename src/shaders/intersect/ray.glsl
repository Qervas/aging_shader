#ifndef RAY_GLSL
#define RAY_GLSL

#include "../common/structures.glsl"

Ray createRay(vec3 origin, vec3 direction) {
    Ray ray;
    ray.origin = origin;
    ray.direction = normalize(direction);
    return ray;
}

#endif // RAY_GLSL
