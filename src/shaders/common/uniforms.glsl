#ifndef UNIFORMS_GLSL
#define UNIFORMS_GLSL

struct WaterTrail {
    vec3 position;
    float intensity;
    float time;
};

layout(binding = 1) uniform sampler2D paintingTexture;
layout(std430, binding = 1) buffer TrailBuffer {
    WaterTrail waterTrails[];
};
uniform float rustLevel;
uniform float age;
uniform float frameWidth;
uniform vec3 cameraPosition;
uniform vec3 cameraFront;
uniform vec3 cameraUp;
uniform int numObjects;
uniform float moisture;
uniform vec3 lightDirection;
uniform float lightIntensity;
uniform float iTime;

uniform int numTrails;
#endif // UNIFORMS_GLSL
