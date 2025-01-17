#pragma once
#include <glm/glm.hpp>

enum class ObjectType {
    SPHERE,
    RECTANGLE,
    GROUND
};

struct SceneObject {
    ObjectType type;
    glm::vec3 position;
    glm::vec3 velocity{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 rotation{0.0f};
    float rustLevel{0.0f};
    bool isDynamic{false};

    SceneObject(ObjectType t, const glm::vec3& pos)
        : type(t), position(pos) {}
};
