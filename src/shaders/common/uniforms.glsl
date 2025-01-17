#ifndef UNIFORMS_GLSL
#define UNIFORMS_GLSL

layout(binding = 1) uniform sampler2D paintingTexture;

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

#endif // UNIFORMS_GLSL
