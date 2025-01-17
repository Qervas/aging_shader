#pragma once
#include "core/scene_object.hpp"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
class Physics;
class Scene {
public:
    Scene(Physics& physics);

    void update(float deltaTime);
    size_t addObject(ObjectType type, const glm::vec3& position, bool isDynamic = false);
    void removeObject(size_t id);
    const SceneObject& getObject(size_t id) const;
    SceneObject& getObject(size_t id);
    const std::vector<SceneObject>& getObjects() const { return objects; }

private:
    std::vector<SceneObject> objects;
    Physics& physics;
};
