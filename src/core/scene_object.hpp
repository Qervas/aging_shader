#pragma once
#include <glm/glm.hpp>
#include <vector>

enum class ObjectType {
    SPHERE,
    RECTANGLE,
    GROUND,
    WALL
};

struct EnvironmentParams {
    float humidity;
    float temperature;
    float salinity;
    float timeOfDay;
};

struct WaterTrail {
    glm::vec3 position;
    float intensity;
    float time;
};


struct SceneObject {
    ObjectType type;
    glm::vec3 position;
    glm::vec3 velocity{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 rotation{0.0f};
    float rustLevel{0.0f};
    bool isDynamic{false};
    std::vector<WaterTrail> waterTrails;
    float lastTrailTime{0.0f};
    struct AgingProperties {
        float exposure;
        float resistance;
        float currentAge;
        std::vector<glm::vec3> stressPoints;
    } agingProps;

    void updateAging(const EnvironmentParams& env, float deltaTime);
    void addWaterTrail(const glm::vec3& pos, float intensity) {
        waterTrails.push_back({pos, intensity, 0.0f});
    }
    SceneObject(ObjectType t, const glm::vec3& pos)
        : type(t), position(pos) {}
};
