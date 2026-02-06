

#include "RacingGame.hpp"

RacingGame::RacingGame()
{
    // Initialize smart pointers for game systems
    renderer = std::make_unique<RenderingSystem>();
    physicsSystem = std::make_unique<PhysicsSystem>();
    textSystem = std::make_unique<Text>();

    textSystem->setProjection(1440.0f, 1440.0f);
}


/// Main game loop
void RacingGame::run()
{
    while (!renderer->shouldClose())
    {
        gameTime.update();

        // Physics System Loop, adaptive based on performance
        int maxPhysicsSteps = gameTime.maxPhysicsSteps();
        int physicsSteps = 0;
        while (gameTime.accumulator >= gameTime.dt && physicsSteps < maxPhysicsSteps) {
            if (gameTime.frameCount < 600) {
                break; // Skip the first frames to avoid slow startup
            }
            physicsSystem->update(gameTime.dtF());
            gameTime.physicsUpdate();
            physicsSteps++;
        }
        
        // Discard excess time when running slow to prevent spiral of death
        if (physicsSteps >= maxPhysicsSteps) {
            gameTime.discardExcessTime();
        }

        renderer->update(gameTime.fpsF());
        
        // Render physics entities
        renderer->renderEntities(physicsSystem->entityList);
        
        renderer->updateUI();

        // Must be called after renderer update, but before text rendering
        textSystem->beginText();

        textSystem->renderText("Hello!",
            { 100.f, 1200.f, 1.f }, { 0.5f, 0.8f, 0.2f });

        textSystem->renderText(
            "Rendered Frames: " + std::to_string(gameTime.frameCount),
            { 100.f, 1150.f, 0.75f }, { 0.2f, 0.5f, 0.8f });

        textSystem->renderText(
            "Physics Frames: " + std::to_string(gameTime.physicsFrameCount),
            { 100.f, 1100.f, 0.75f }, { 0.5f, 0.2f, 0.8f });

        textSystem->renderText(
            "Game FPS: " + std::to_string(1.0f / gameTime.fpsF()),
            { 100.f, 1050.f, 0.75f }, { 0.8f, 0.8f, 0.2f });

        textSystem->endText();

        // Must be called last
        renderer->endFrame();
    }
}
