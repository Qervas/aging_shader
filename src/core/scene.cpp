#include "scene.hpp"
#include "physics.hpp"
#include <GLFW/glfw3.h>

Scene::Scene(Physics& physics) : physics(physics) {
    // Add ground as a default object
    addObject(ObjectType::GROUND, glm::vec3(0.0f, -1.0f, 0.0f), false);
    physics.setSceneObjects(objects);

}

void Scene::update(float deltaTime) {
    // Create environment parameters based on time of day
    EnvironmentParams env;
    env.timeOfDay = static_cast<float>(fmod(glfwGetTime(), 24.0));
    env.humidity = 0.5f; // Base humidity
    env.temperature = 20.0f; // Base temperature
    env.salinity = 0.1f; // Base salinity

    for (auto& obj : objects) {
        // Update physics for dynamic objects
        if (obj.isDynamic) {
            physics.updateObject(obj);
        }

        // Update aging for all objects
        obj.updateAging(env, deltaTime);
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
