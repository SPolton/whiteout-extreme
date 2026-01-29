

#include "RacingGame.h"
#include "physics/PhysicsTest.h"

void RacingGame::run()
{
    // Initialize smart pointers for game systems
    renderer = std::make_unique<RenderingSystem>();
    physicsSystem = std::make_unique<PhysicsSystem>();
    textSystem = std::make_unique<Text>();

    while (!renderer->shouldClose())
    {
        gameTime.update();

        // Physics System Loop
        while (gameTime.accumulator >= gameTime.dt) {
            physicsSystem->update(gameTime.dt);
            gameTime.physicsUpdate();
        }

        renderer->update();
        renderer->updateUI();

        // Must be called after renderer update, but before text rendering
        textSystem->update();

        textSystem->renderText(
            "Rendered Frames: " + std::to_string(gameTime.frameCount),
            { 100.f, 1150.f, 0.75f },
            glm::vec3(0.2f, 0.5f, 0.8f)
        );

        textSystem->renderText(
            "Physics Frames: " + std::to_string(gameTime.physicsFrameCount),
            { 100.f, 1100.f, 0.75f },
            glm::vec3(0.5f, 0.2f, 0.8f)
        );

        textSystem->renderText(
            "Game FPS: " + std::to_string(1 / gameTime.fps),
            { 100.f, 1050.f, 0.75f },
            glm::vec3(0.8f, 0.8f, 0.2f)
        );

        // Must be called last
        renderer->endFrame();
    }
}
