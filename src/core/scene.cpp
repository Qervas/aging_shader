#include "scene.hpp"
#include "physics.hpp"

Scene::Scene(Physics& physics) : physics(physics) {
    // Add ground as a default object
    addObject(ObjectType::GROUND, glm::vec3(0.0f, -1.0f, 0.0f), false);
    physics.setSceneObjects(objects);

}

void Scene::update([[maybe_unused]] float deltaTime) {
    for (auto& obj : objects) {
        if (obj.isDynamic) {
            physics.updateObject(obj);
        }
    }
}

size_t Scene::addObject(ObjectType type, const glm::vec3& position, bool isDynamic) {
    objects.emplace_back(type, position);
    objects.back().isDynamic = isDynamic;
    return objects.size() - 1;
}

void Scene::removeObject(size_t id) {
    if (id < objects.size()) {
        objects.erase(objects.begin() + id);
    }
}

const SceneObject& Scene::getObject(size_t id) const {
    return objects.at(id);
}

SceneObject& Scene::getObject(size_t id) {
    return objects.at(id);
}
