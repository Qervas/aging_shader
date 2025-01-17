#include "scene_object.hpp"
#include <algorithm>

void SceneObject::updateAging(const EnvironmentParams& env, float deltaTime) {
    // Update aging based on environment
    agingProps.currentAge += deltaTime * (
        env.humidity * 0.3f +    // Moisture accelerates aging
        env.temperature * 0.2f + // Higher temperature accelerates aging
        env.salinity * 0.5f      // Salt accelerates aging
    );
}
