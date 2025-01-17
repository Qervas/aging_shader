#include "scene_object.hpp"
#include <algorithm>

void SceneObject::updateAging(const EnvironmentParams& env, float deltaTime) {
    // Update water trails
    for (auto it = waterTrails.begin(); it != waterTrails.end();) {
        it->time += deltaTime;
        if (it->time > 2.0f) { // Remove trails after 2 seconds
            it = waterTrails.erase(it);
        } else {
            ++it;
        }
    }

    // Update aging based on environment
    agingProps.currentAge += deltaTime * (
        env.humidity * 0.3f +    // Moisture accelerates aging
        env.temperature * 0.2f + // Higher temperature accelerates aging
        env.salinity * 0.5f      // Salt accelerates aging
    );

    // Limit maximum trails to prevent memory issues
    if (waterTrails.size() > 100) {
        waterTrails.erase(waterTrails.begin(), waterTrails.begin() + 50);
    }
}
