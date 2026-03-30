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
        {{-293.824f, 611.567f, -279.847f}, //1
        {0.752773f, -0.0136276f, 0.65799f, -0.0139685f}},
        {{-396.285f, 589.239f, -389.207f}, //2
        {-0.929244f, 0.0926873f, 0.351035f, -0.0684793f}},
        {{-581.787f, 556.707f, -571.819f}, //3
        {0.252806f, -0.0274139f, -0.967111f, 0.00579301f}},
        {{-409.932f, 536.539f, -641.265f}, //4
        {-0.350299f, -0.0438172f, -0.935238f, -0.0264539f}},
        {{-148.029f, 505.572f, -393.93f}, //5
        {-0.726361f, -0.0504391f, -0.685341f, -0.0127893f}},
        {{-223.837f, 484.827f, -178.762f}, //6
        {-0.94168f, -0.0114114f, 0.323926f, 0.0904444f}},
        {{-61.3555f, 452.522f, -115.411f}, //7
        {-0.843571f, -0.0370263f, -0.535389f, 0.0193785f}},
        {{168.599f, 425.777f, -47.2245f}, //8
        {-0.76879f, 0.0248985f, -0.63583f, -0.063734f}},
        {{-32.741f, 405.57f, 80.7205f}, //9
        {-0.566188f, -0.0433537f, 0.823133f, -0.0020807f}},
        {{-202.497f, 390.646f, 110.79f}, //10
        {-0.817959f, 0.0301062f, 0.574259f, -0.0162499f}},
        {{-229.103f, 352.083f, -82.2011f}, //11
        {0.150352f, -0.127507f, 0.979292f, -0.0460913f}},
        {{-499.641f, 299.978f, -169.357f}, //12
        {-0.400111f, -0.0144815f, 0.916348f, 0.00280296f}},
        {{-506.09f, 298.874f, -79.727f}, //13
        {-0.991013f, -0.000953557f, 0.129251f, -0.0344371f}},
        {{-732.649f, 279.788f, -77.9927f}, //14
        {-0.7386f, -0.0120449f, 0.673774f, 0.0188001f}},
        {{-674.788f, 271.716f, 109.215f}, //15
        {-0.878127f, 0.0210859f, -0.476935f, -0.031341f}},
        {{-583.666f, 269.141f, 312.304f}, //16
        {-0.992259f, 0.0391282f, -0.117636f, 0.00732213f}},
        {{-312.633f, 248.263f, 199.152f}, //17
        {-0.369061f, -0.0312248f, -0.928262f, -0.0338999f}},
        {{10.1303f, 155.399f, 174.775f}, //18
        {-0.45224f, -0.215606f, -0.863206f, -0.0621973f}}
    };
};
