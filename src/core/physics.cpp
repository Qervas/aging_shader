#include "physics.hpp"
#include "scene.hpp"
#include <GLFW/glfw3.h>

Physics::Physics(const std::vector<SceneObject>& objects)
    : objectsInScene(&objects) {
    // Initialize sphere in the air
    spherePosition = glm::vec3(0.0f, 5.0f, -1.0f);
    sphereVelocity = glm::vec3(0.0f);
}

void Physics::update(float deltaTime) {
    // Apply gravity to camera
    if (!cameraGrounded) {
        cameraVelocity.y += GRAVITY * deltaTime;
    }

    // Apply friction to camera when grounded
    if (cameraGrounded) {
        float friction = FRICTION * deltaTime;
        cameraVelocity.x *= std::max(0.0f, 1.0f - friction);
        cameraVelocity.z *= std::max(0.0f, 1.0f - friction);
    } else {
        // Apply air resistance
        cameraVelocity *= (1.0f - AIR_RESISTANCE * deltaTime);
    }

    // Handle camera collisions
    handleCollisions();

    // Update all scene objects
    if (objectsInScene) {
        for (auto& obj : *const_cast<std::vector<SceneObject>*>(objectsInScene)) {
            if (obj.isDynamic) {
                updateObject(obj);
            }
        }
    }
}

void Physics::updateCameraPosition(glm::vec3& position) {
    glm::vec3 newPos = position + cameraVelocity * 0.016f;
    bool collision = false;

    // Check sphere collision first
    glm::vec3 toSphere = spherePosition - newPos;
    float distance = glm::length(toSphere);

    if (distance < (CAMERA_RADIUS + SPHERE_RADIUS)) {
        // Collision detected
        glm::vec3 normal = glm::normalize(toSphere);
        float penetration = CAMERA_RADIUS + SPHERE_RADIUS - distance;

        // Move objects apart
        newPos -= normal * penetration * 0.5f;
        spherePosition += normal * penetration * 0.5f;

        // Calculate impulse
        float impulseMagnitude = glm::length(cameraVelocity) * 1.5f;
        sphereVelocity += normal * impulseMagnitude;
        cameraVelocity = cameraVelocity * 0.5f; // Reduce camera velocity

        collision = true;
    }

    // Check ground collision
    if (checkGroundCollision(newPos, CAMERA_HEIGHT)) {
        newPos.y = GROUND_Y + CAMERA_HEIGHT * 0.5f;
        if (cameraVelocity.y < 0) {
            cameraVelocity.y = 0;
        }
        collision = true;
    }

    // Update position
    position = newPos;
    cameraPosition = position;

    // Update grounded state
    cameraGrounded = checkGroundCollision(position, CAMERA_HEIGHT + 0.1f);
}

void Physics::applyCameraForce(const glm::vec3& force) {
    cameraVelocity += force;
}

void Physics::handleCollisions() {
    // Ground collision
    float bottomY = spherePosition.y - SPHERE_RADIUS;
    if (bottomY < GROUND_Y) {
        spherePosition.y = GROUND_Y + SPHERE_RADIUS;
        if (sphereVelocity.y < 0) {
            // Add some horizontal momentum loss on impact
            sphereVelocity.y = -sphereVelocity.y * RESTITUTION;
            sphereVelocity.x *= 0.9f;
            sphereVelocity.z *= 0.9f;
        }
    }

    // Wall collisions
    const float WALL_Z = -5.0f;
    const float WALL_HALF_WIDTH = 5.0f;

    // Side walls with more bounce
    if (std::abs(spherePosition.x) > WALL_HALF_WIDTH - SPHERE_RADIUS) {
        spherePosition.x = std::copysign(WALL_HALF_WIDTH - SPHERE_RADIUS, spherePosition.x);
        sphereVelocity.x = -sphereVelocity.x * RESTITUTION;
    }

    // Back wall
    if (spherePosition.z < WALL_Z + SPHERE_RADIUS) {
        spherePosition.z = WALL_Z + SPHERE_RADIUS;
        sphereVelocity.z = -sphereVelocity.z * RESTITUTION;
    }
}
bool Physics::checkSphereCollision(const glm::vec3& position, float radius) {
    float distance = glm::length(position - spherePosition);
    if (distance < (radius + SPHERE_RADIUS)) {
        glm::vec3 normal = glm::normalize(spherePosition - position);
        float overlap = (radius + SPHERE_RADIUS) - distance;

        // Move sphere away from collision
        spherePosition += normal * overlap * 0.5f;

        // Apply impulse based on camera velocity
        float impulseMagnitude = glm::length(cameraVelocity) * 0.8f;
        sphereVelocity += normal * impulseMagnitude;

        return true;
    }
    return false;
}

bool Physics::checkGroundCollision(const glm::vec3& position, float height) {
    float groundHeight = GROUND_Y - 0.5f *
        std::exp(-glm::length(glm::vec2(position.x, position.z)) * 1.5f);
    return position.y - height/2.0f < groundHeight;
}

void Physics::updateObject(SceneObject& obj) {
    if (!obj.isDynamic) return;

    if (obj.type == ObjectType::SPHERE) {
        // Sync sphere position and velocity with Physics system
        obj.position = spherePosition;
        obj.velocity = sphereVelocity;
    }

    updateDynamicObject(obj, 0.016f);

    if (obj.type == ObjectType::SPHERE) {
        // Update Physics system with new position and velocity
        spherePosition = obj.position;
        sphereVelocity = obj.velocity;
    }
}

void Physics::handleObjectCollisions(SceneObject& obj) {
    // Ground collision
    float bottomY;
    float radius;
    const float WALL_RESTITUTION = 0.5f;
    const float WALL_Z = -5.0f;
    const float WALL_HALF_WIDTH = 5.0f;
    const float WALL_HEIGHT = 5.0f;

    switch(obj.type) {
        case ObjectType::SPHERE:
            radius = SPHERE_RADIUS;

            // Ground collision
            bottomY = obj.position.y - radius;
            if (bottomY < GROUND_Y) {
                obj.position.y = GROUND_Y + radius;
                if (obj.velocity.y < 0) {
                    obj.velocity.y = -obj.velocity.y * RESTITUTION;

                    // Add water trail when sphere hits ground
                    float impactSpeed = glm::length(obj.velocity);
                    if (impactSpeed > 0.5f) { // Increased threshold
                        float intensity = glm::clamp(impactSpeed / 15.0f, 0.0f, 0.8f); // Reduced max intensity
                        obj.addWaterTrail(obj.position - glm::vec3(0.0f, radius - 0.01f, 0.0f), intensity);
                    }
                }
            }
            // Wall collision
            if (std::abs(obj.position.x) > WALL_HALF_WIDTH) {
                // Side walls
                obj.position.x = std::copysign(WALL_HALF_WIDTH, obj.position.x);
                obj.velocity.x = -obj.velocity.x * WALL_RESTITUTION;
            }

            if (obj.position.z <= WALL_Z + radius) {
                // Back wall
                obj.position.z = WALL_Z + radius;
                obj.velocity.z = -obj.velocity.z * WALL_RESTITUTION;
            }

            // Ceiling collision
            if (obj.position.y + radius > WALL_HEIGHT) {
                obj.position.y = WALL_HEIGHT - radius;
                obj.velocity.y = -obj.velocity.y * WALL_RESTITUTION;
            }
            break;

        case ObjectType::RECTANGLE:
            // For rectangles (like paintings), prevent them from going through walls
            if (obj.position.z <= WALL_Z + 0.1f) {
                obj.position.z = WALL_Z + 0.1f;
                obj.velocity = glm::vec3(0.0f); // Stop movement
            }
            if (std::abs(obj.position.x) > WALL_HALF_WIDTH - 1.0f) {
                obj.position.x = std::copysign(WALL_HALF_WIDTH - 1.0f, obj.position.x);
                obj.velocity.x = 0.0f;
            }
            if (obj.position.y < GROUND_Y) {
                obj.position.y = GROUND_Y;
                obj.velocity.y = 0.0f;
            }
            if (obj.position.y > WALL_HEIGHT - 1.0f) {
                obj.position.y = WALL_HEIGHT - 1.0f;
                obj.velocity.y = 0.0f;
            }
            break;

        case ObjectType::GROUND:
            // Ground is static, no collision handling needed
            break;

        case ObjectType::WALL:
            // Walls are static, no collision handling needed
            break;
    }

    // Object-object collisions
    for (const auto& other : *objectsInScene) {
        if (&other == &obj) continue;

        if (obj.type == ObjectType::SPHERE && other.type == ObjectType::SPHERE) {
            // Sphere-sphere collision
            glm::vec3 diff = obj.position - other.position;
            float dist = glm::length(diff);
            float minDist = SPHERE_RADIUS * 2.0f;

            if (dist < minDist) {
                // Collision response
                glm::vec3 normal = glm::normalize(diff);
                float overlap = minDist - dist;

                // Separate spheres
                obj.position += normal * (overlap * 0.5f);

                // Calculate new velocities
                glm::vec3 relativeVel = obj.velocity - other.velocity;
                float normalVel = glm::dot(relativeVel, normal);

                if (normalVel < 0.0f) {
                    float j = -(1.0f + RESTITUTION) * normalVel;
                    obj.velocity += normal * j * 0.5f;
                }
            }
        }
        else if (obj.type == ObjectType::SPHERE && other.type == ObjectType::RECTANGLE) {
            // Simple sphere-rectangle collision (basic AABB check)
            glm::vec3 closest = glm::clamp(
                obj.position,
                other.position - glm::vec3(1.0f, 1.0f, 0.1f),
                other.position + glm::vec3(1.0f, 1.0f, 0.1f)
            );

            glm::vec3 diff = obj.position - closest;
            float dist = glm::length(diff);

            if (dist < SPHERE_RADIUS) {
                // Collision response
                glm::vec3 normal = glm::normalize(diff);
                obj.position = closest + normal * SPHERE_RADIUS;

                // Reflect velocity
                float normalVel = glm::dot(obj.velocity, normal);
                if (normalVel < 0.0f) {
                    obj.velocity = glm::reflect(obj.velocity, normal) * WALL_RESTITUTION;
                }
            }
        }
    }
}

void Physics::updateDynamicObject(SceneObject& obj, float deltaTime) {
    // Apply gravity
    obj.velocity.y += GRAVITY * deltaTime;

    // Update position
    obj.position += obj.velocity * deltaTime;

    // Handle collisions
    handleObjectCollisions(obj);

    // Apply air resistance
    obj.velocity *= (1.0f - AIR_RESISTANCE * deltaTime);
}
