#include "physics.hpp"
#include "scene.hpp"
Physics::Physics() {}

void Physics::update(float deltaTime) {
    // Apply gravity to both sphere and camera
    sphereVelocity.y += GRAVITY * deltaTime;
    if (!cameraGrounded) {
        cameraVelocity.y += GRAVITY * deltaTime;
    }

    // Update sphere position
    spherePosition += sphereVelocity * deltaTime;

    // Apply friction to camera when grounded
    if (cameraGrounded) {
        float friction = FRICTION * deltaTime;
        cameraVelocity.x *= std::max(0.0f, 1.0f - friction);
        cameraVelocity.z *= std::max(0.0f, 1.0f - friction);
    } else {
        // Apply air resistance
        cameraVelocity *= (1.0f - AIR_RESISTANCE * deltaTime);
    }

    handleCollisions();
}

void Physics::updateCameraPosition(glm::vec3& position) {
    glm::vec3 newPos = position + cameraVelocity * 0.016f; // Assuming 60 FPS

    // Check for collisions at new position
    bool collision = checkGroundCollision(newPos, CAMERA_HEIGHT) ||
                    checkSphereCollision(newPos, CAMERA_RADIUS);

    if (!collision) {
        position = newPos;
    } else {
        // Handle collision response
        cameraVelocity = glm::vec3(0.0f);
    }

    // Update grounded state
    cameraGrounded = checkGroundCollision(position, CAMERA_HEIGHT + 0.1f);
}

void Physics::applyCameraForce(const glm::vec3& force) {
    cameraVelocity += force;
}

void Physics::handleCollisions() {
    // Ground collision for sphere
    float bottomY = spherePosition.y - SPHERE_RADIUS;
    if (bottomY < GROUND_Y) {
        spherePosition.y = GROUND_Y + SPHERE_RADIUS;
        if (sphereVelocity.y < 0) {
            sphereVelocity.y = -sphereVelocity.y * RESTITUTION;
        }
    }
}

bool Physics::checkSphereCollision(const glm::vec3& position, float radius) {
    // Check collision with the metal sphere
    float distance = glm::length(position - spherePosition);
    return distance < (radius + SPHERE_RADIUS);
}

bool Physics::checkGroundCollision(const glm::vec3& position, float height) {
    return position.y - height/2.0f < GROUND_Y;
}

void Physics::updateObject(SceneObject& obj) {
    if (!obj.isDynamic) return;

    updateDynamicObject(obj, 0.016f); // Using fixed timestep for now
}

void Physics::handleObjectCollisions(SceneObject& obj) {
    // Ground collision
    float bottomY;
    float radius;

    switch(obj.type) {
        case ObjectType::SPHERE:
            radius = SPHERE_RADIUS; // You might want to make this configurable per object
            bottomY = obj.position.y - radius;
            if (bottomY < GROUND_Y) {
                obj.position.y = GROUND_Y + radius;
                if (obj.velocity.y < 0) {
                    obj.velocity.y = -obj.velocity.y * RESTITUTION;
                }
            }
            break;

        case ObjectType::RECTANGLE:
            // Handle rectangle collisions if needed
            break;

        case ObjectType::GROUND:
            // Ground doesn't need collision handling
            break;
    }

    // Object-object collisions could be added here
}

void Physics::updateDynamicObject(SceneObject& obj, float deltaTime) {
    // Apply gravity
    obj.velocity.y += GRAVITY * deltaTime;

    // Update position
    obj.position += obj.velocity * deltaTime;

    // Handle collisions
    handleObjectCollisions(obj);
}
