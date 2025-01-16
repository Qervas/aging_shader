#ifndef UTILS_GLSL
#define UTILS_GLSL

vec3 tonemap(vec3 color) {
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Gamma correction
    return pow(color, vec3(1.0 / 2.2));
}

#endif // UTILS_GLSL
