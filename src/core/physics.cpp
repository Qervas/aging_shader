#include "physics.hpp"
#include "scene.hpp"


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
}
