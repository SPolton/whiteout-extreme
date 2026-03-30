#pragma once

#include <memory>
#include "ecs/Coordinator.hpp"

#include "core/RenderingSystem.hpp"
#include "input/VehicleControlSystem.hpp"

#include "components/VehicleComponent.h"
#include "components/SnowCannon.h"
#include "components/SnowBall.h"


class SnowBallisticSystem : public System {
public:
    SnowBallisticSystem(std::shared_ptr<RenderingSystem> renderingSystem,
        std::shared_ptr<VehicleControlSystem> vehicleControlSystem);

    void init();
    void update(float deltaTime);

private:
    std::shared_ptr<RenderingSystem> renderingSystem;
    std::shared_ptr<VehicleControlSystem> vehicleControlSystem;

    void setupSnowCannonsFromPosAndRot();
    void setupSnowCannonEntity(glm::vec3 position, glm::quat rotation);

    std::vector<SnowCannon> snowCannons;

    std::vector<std::pair<glm::vec3, glm::quat>> snowCannonsInitialPosAndRot = {
        //1
        {
            {-766.474f, 650.422f, -377.178f},
            {0.665715f, 0.0731619f, -0.716646f, 0.194652f}
        },
        //2
        {
            {-678.7f, 643.416f, -328.694f},
            {0.974737f, -0.0356654f, -0.216141f, -0.0435823f}
        }
    };
};
