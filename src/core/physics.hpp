#pragma once
#include "core/scene.hpp"
#include <glm/glm.hpp>
#include "scene_object.hpp"

class Physics {
public:
    Physics();
    void update(float deltaTime);
    void updateObject(SceneObject& obj);

    const glm::vec3& getSpherePosition() const { return spherePosition; }
    const glm::vec3& getSphereVelocity() const { return sphereVelocity; }
    void setSpherePosition(const glm::vec3& pos) { spherePosition = pos; }
    void setSphereVelocity(const glm::vec3& vel) { sphereVelocity = vel; }

    // Camera physics
    const glm::vec3& getCameraVelocity() const { return cameraVelocity; }
    void setCameraVelocity(const glm::vec3& vel) { cameraVelocity = vel; }
    void applyCameraForce(const glm::vec3& force);
    bool isCameraGrounded() const { return cameraGrounded; }
    void updateCameraPosition(glm::vec3& position);


private:
    glm::vec3 spherePosition{0.0f, 0.0f, -1.0f};
    glm::vec3 sphereVelocity{0.0f, 0.0f, 0.0f};
    const float SPHERE_RADIUS = 1.0f;

    glm::vec3 cameraVelocity{0.0f, 0.0f, 0.0f};
    bool cameraGrounded = false;
    const float CAMERA_HEIGHT = 1.8f;  // Standing height
    const float CAMERA_RADIUS = 0.3f;
    // Physics constants
    const float GRAVITY = -9.81f;
    const float GROUND_Y = -1.0f;
    const float RESTITUTION = 0.3f;
    const float FRICTION = 8.0f;
    const float AIR_RESISTANCE = 0.5f;

    void handleCollisions();
    bool checkSphereCollision(const glm::vec3& position, float radius);
    bool checkGroundCollision(const glm::vec3& position, float height);
    void updateDynamicObject(SceneObject& obj, float deltaTime);
    void handleObjectCollisions(SceneObject& obj);
};
