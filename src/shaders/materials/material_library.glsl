#ifndef MATERIAL_LIBRARY_GLSL
#define MATERIAL_LIBRARY_GLSL

#include "../common/structures.glsl"

Material createGoldMaterial() {
    return Material(
        vec3(1.0, 0.765, 0.336), // albedo
        1.0, // metallic
        0.1, // roughness
        0.47 // IOR
    );
}

Material createGroundMaterial() {
    return Material(
        vec3(0.2, 0.2, 0.2), // albedo
        0.0, // metallic
        0.8, // roughness
        1.5 // IOR
    );
}

#endif // MATERIAL_LIBRARY_GLSL
