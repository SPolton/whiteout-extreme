#include "RenderingSystem.hpp"
#include "utils/logger.h"
#include "core/render/ShapeGenerator.hpp"

#include "ecs/Coordinator.hpp"
#include "components/Transform.h"
#include "components/VehicleComponent.h"

#include <glm/gtc/type_ptr.hpp>

using namespace render;

RenderingSystem::RenderingSystem(
    std::shared_ptr<InputManager> inputManager)
    : inputManager(inputManager)
{
    if (!init()) {
        throw std::runtime_error("Failed to initialize RenderingSystem!");
    }
}

RenderingSystem::~RenderingSystem()
{
    releaseShadowDepthResources();
}

bool RenderingSystem::init()
{
    logger::info("OpenGL Version: {0}", (const char*)glGetString(GL_VERSION));
    logger::info("OpenGL Renderer: {0}", (const char*)glGetString(GL_RENDERER));

    // Load depth shader used by the shadow depth pass.
    try
    {
        depthShader = assetManager.loadShader("depth");
        logger::info("Depth shader loaded successfully");
    }
    catch (const std::exception& e)
    {
        logger::error("Failed to load depth shader: {0}", e.what());
        return false;
    }

    // Create object tracking transform for camera (vehicle tracking)
    targetTransform = std::make_unique<SceneTransform>();
    targetTransform->setPosition(glm::vec3(0.f, 0.f, 0.f));

    // Create cameras
    racingCamera = std::make_unique<RacingCamera>();
    turntableCamera = std::make_unique<TurnTableCamera>(*targetTransform);
    turntableCamera->adjustTheta(glm::radians(180.f));
    turntableCamera->adjustDistance(3.f);
    freeCamera = std::make_unique<FreeCamera>();
    activeCamera = racingCamera.get();  // Non-owning raw pointer to camera
    
    logger::info("Camera initialized");

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    return true;
}

/* ----- Entity creation ----- */

Renderable RenderingSystem::getCubeRenderable(const std::string& texturePath)
{
    return Renderable{
        .geometry = assetManager.loadGeometry("cube", ShapeGenerator::cube()),
        .cpuData = assetManager.getCPUGeometry("cube"),
        .material = RenderMaterial{
            .shader = assetManager.loadShader("textured"),
            .baseTexture = assetManager.loadTexture(texturePath, GL_LINEAR),
        }
    };
}

Entity RenderingSystem::createPlaneEntity(const std::string& texturePath, const PlaneConfig& config)
{
    Entity plane = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(
        plane,
        PhysxTransform{
            config.position,
            config.rotation,
            config.scale
        }
    );

    std::string geomKey;
    CPU_Geometry planeCPU;
    
    if (config.isInfinite) {
        geomKey = "infinite_plane";
        planeCPU = ShapeGenerator::infinitePlane(config.infinitePlaneSize, config.uvRepeat);
        logger::info("Infinite plane entity created with UV repeat: {}", config.uvRepeat);
    } else {
        geomKey = "plane";
        planeCPU = ShapeGenerator::plane(config.size);
        logger::info("Plane entity created with size: {}", config.size);
    }

    gCoordinator.AddComponent(
        plane,
        Renderable{
            .geometry = assetManager.loadGeometry(geomKey, planeCPU),
            .cpuData = assetManager.getCPUGeometry(geomKey),
            .material = RenderMaterial{
                .shader = assetManager.loadShader("textured"),
                .baseTexture = assetManager.loadTexture(texturePath, config.textureFilterMode, config.textureWrapMode),
            }
        }
    );

    return plane;
}

Entity RenderingSystem::createBoxEntity(const std::string& texturePath, const render::BoxConfig& config)
{
    Entity box = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(
        box,
        PhysxTransform{
            config.position,
            config.rotation,
            config.scale
        }
    );

    gCoordinator.AddComponent(
        box,
        getCubeRenderable(texturePath)
    );

    logger::info("Box entity created at ({}, {}, {})", config.position.x, config.position.y, config.position.z);
    return box;
}

Entity RenderingSystem::createSphereEntity(const std::string& texturePath, const SphereConfig& config)
{
    Entity sphere = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(
        sphere,
        PhysxTransform{
            config.position,
            config.rotation,
            config.scale
        }
    );

    // Generate geometry key based on config
    std::string geomKey = config.isSkybox ? "skybox" : "sphere";
    
    // Create geometry with specified parameters
    CPU_Geometry sphereCPU = ShapeGenerator::sphere(config.radius, config.slices, config.stacks);
    
    gCoordinator.AddComponent(
        sphere,
        Renderable{
            .geometry = assetManager.loadGeometry(geomKey, sphereCPU),
            .cpuData = assetManager.getCPUGeometry(geomKey),
            .material = RenderMaterial{
                .shader = assetManager.loadShader("textured"),
                .baseTexture = assetManager.loadTexture(texturePath, config.textureFilterMode, config.textureWrapMode),
            },
            .isSkybox = config.isSkybox
        }
    );

    logger::info("Sphere entity created (radius={}, skybox={}) with texture: {}", 
                 config.radius, config.isSkybox, texturePath);

    return sphere;
}

Entity RenderingSystem::createModelEntity(const std::string& modelPath, const ModelConfig& config)
{
    Entity model = gCoordinator.CreateEntity();

    gCoordinator.AddComponent(
        model,
        PhysxTransform{
            config.position,
            config.rotation,
            config.scale
        }
    );

    try {
        auto modelLoader = std::make_shared<ModelLoader>(modelPath, false);
        logger::info("Model loaded successfully: {} with {} meshes", modelPath, modelLoader->getMeshCount());
        
        gCoordinator.AddComponent(
            model,
            ModelRenderable{
                .modelLoader = modelLoader,
                .material = RenderMaterial{
                    .shader = assetManager.loadShader("model"),
                    .useTextureScroll = false,
                    .useModelLighting = true
                }
            }
        );
    }
    catch (const std::exception& e) {
        logger::error("Failed to load model {}: {}", modelPath, e.what());
        throw;
    }

    logger::info("Model entity created: {}", modelPath);

    return model;
}

/* ----- Render step ----- */

RenderFrameContext RenderingSystem::buildFrameContext() const
{
    const float fov = activeCamera->fov();
    const float aspectRatio = static_cast<float>(vWidth) / static_cast<float>(vHeight);
    const float nearPlane = 0.1f;
    const float farPlane = 5000.0f;

    RenderFrameContext frameContext{};
    frameContext.view = activeCamera->viewMatrix();
    frameContext.projection = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
    frameContext.cameraPosition = activeCamera->position();
    frameContext.viewportSize = glm::vec2(static_cast<float>(vWidth), static_cast<float>(vHeight));
    frameContext.lighting = lightingState;

    if (frameContext.lighting.shadowsEnabled) {
        glm::vec3 const lightDirection = glm::normalize(frameContext.lighting.lightDirection);
        // Keep the sun shadow frustum anchored in world space (not camera space).
        glm::vec3 const lightTarget = frameContext.lighting.lightPosition;
        glm::vec3 const lightPosition = lightTarget - lightDirection * 150.0f;

        glm::mat4 const lightView = glm::lookAt(
            lightPosition,
            lightTarget,
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        float const range = frameContext.lighting.shadowOrthoRange;
        glm::mat4 const lightProjection = glm::ortho(
            -range,
            range,
            -range,
            range,
            frameContext.lighting.shadowNearPlane,
            frameContext.lighting.shadowFarPlane
        );

        frameContext.lighting.lightPosition = lightPosition;
        frameContext.lighting.lightViewProjection = lightProjection * lightView;
    }

    return frameContext;
}

void RenderingSystem::render()
{
    statsData.startFrame();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderFrameContext const frameContext = buildFrameContext();

    snowRenderer.beginFrame();

    renderShadowDepthPass(frameContext);
    renderGeometryPass(frameContext);
    renderModelsPass(frameContext);
    renderParticlesPass(frameContext);

    statsData.endFrame();
}

void RenderingSystem::renderShadowDepthPass(const RenderFrameContext& frameContext)
{
    if (!frameContext.lighting.shadowsEnabled || !depthShader) {
        return;
    }

    ensureShadowDepthResources(
        frameContext.lighting.shadowMapResolution,
        frameContext.lighting.shadowMapResolution
    );

    if (shadowDepthResources.framebuffer == 0 || shadowDepthResources.depthTexture == 0) {
        return;
    }

    // Shadow map uses its own viewport resolution (usually square and higher than window size).
    glViewport(0, 0, shadowDepthResources.width, shadowDepthResources.height);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowDepthResources.framebuffer);
    glClear(GL_DEPTH_BUFFER_BIT);

    depthShader->use();
    // Depth pass only needs light-space transform + model matrix.
    // No color outputs are written.
    glUniformMatrix4fv(
        glGetUniformLocation(*depthShader, "lightViewProjection"),
        1, GL_FALSE, glm::value_ptr(frameContext.lighting.lightViewProjection)
    );

    for (auto const& entity : mEntities)
    {
        if (!gCoordinator.HasComponent<PhysxTransform>(entity)) {
            continue;
        }

        auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);

        if (gCoordinator.HasComponent<Renderable>(entity)) {
            auto& renderable = gCoordinator.GetComponent<Renderable>(entity);

            glm::vec3 localOffset(0.0f);
            if (gCoordinator.HasComponent<VehicleComponent>(entity)) {
                localOffset = glm::vec3(0.0f, 0.75f, 2.0f);
            }

            glm::mat4 const modelMatrix = buildModelMatrix(transform, localOffset);
            glUniformMatrix4fv(
                glGetUniformLocation(*depthShader, "model"),
                1, GL_FALSE, glm::value_ptr(modelMatrix)
            );

            renderable.geometry->bind();

            if (!renderable.cpuData->indices.empty()) {
                glDrawElements(
                    GL_TRIANGLES,
                    static_cast<GLsizei>(renderable.cpuData->indices.size()),
                    GL_UNSIGNED_INT,
                    nullptr
                );
            }
            else {
                glDrawArrays(
                    GL_TRIANGLES,
                    0, static_cast<GLsizei>(renderable.cpuData->positions.size())
                );
            }

            continue;
        }

        if (gCoordinator.HasComponent<ModelRenderable>(entity)) {
            auto& modelRenderable = gCoordinator.GetComponent<ModelRenderable>(entity);
            if (!modelRenderable.modelLoader) {
                continue;
            }

            glm::mat4 const modelMatrix = buildModelMatrix(transform, modelRenderable.visualOffsetPos);
            glUniformMatrix4fv(
                glGetUniformLocation(*depthShader, "model"),
                1, GL_FALSE, glm::value_ptr(modelMatrix)
            );

            modelRenderable.modelLoader->draw(*depthShader);
        }
    }

    // Return to default framebuffer + window viewport for regular forward passes.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, static_cast<int>(frameContext.viewportSize.x), static_cast<int>(frameContext.viewportSize.y));
}

void RenderingSystem::renderGeometryPass(const RenderFrameContext& frameContext)
{
    statsData.startGeometryPass();

    for (auto const& entity : mEntities)
    {
        if (gCoordinator.HasComponent<Renderable>(entity))
        {
            renderRenderableEntity(entity, frameContext);
            statsData.addRenderableDraw();
        }
    }
    statsData.endGeometryPass();
}

void RenderingSystem::renderModelsPass(const RenderFrameContext& frameContext)
{
    statsData.startModelPass();

    for (auto const& entity : mEntities)
    {
        if (gCoordinator.HasComponent<ModelRenderable>(entity))
        {
            renderModelEntity(entity, frameContext);
            statsData.addModelDraw();
        }
    }
    statsData.endModelPass();
}

void RenderingSystem::renderParticlesPass(const RenderFrameContext& frameContext)
{
    statsData.startParticlePass();
    snowRenderer.render(frameContext.view, frameContext.projection);
    statsData.endParticlePass();
}

void RenderingSystem::renderRenderableEntity(Entity entity, const RenderFrameContext& frameContext)
{
    auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);
    auto& renderable = gCoordinator.GetComponent<Renderable>(entity);

    renderable.updateRollingTexture(transform.pos);

    if (renderable.isSkybox) {
        glDepthMask(GL_FALSE); // Don't write to depth buffer
        glFrontFace(GL_CW);    // Reverse winding order to see inside
    }

    glm::vec3 localOffset(0.0f);
    if (gCoordinator.HasComponent<VehicleComponent>(entity)) {
        localOffset = glm::vec3(0.0f, 0.75f, 2.0f);
    }

    bindMaterial(renderable.material);

    glm::mat4 const modelMatrix = buildModelMatrix(transform, localOffset);
    uploadCommonMatrices(renderable.material.shader, modelMatrix, frameContext.view, frameContext.projection);

    renderable.geometry->bind();

    if (!renderable.cpuData->indices.empty()) {
        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(renderable.cpuData->indices.size()),
            GL_UNSIGNED_INT,
            nullptr
        );
    }
    else {
        glDrawArrays(
            GL_TRIANGLES,
            0, static_cast<GLsizei>(renderable.cpuData->positions.size())
        );
    }

    // Restore normal rendering state
    if (renderable.isSkybox) {
        glDepthMask(GL_TRUE);
        glFrontFace(GL_CCW);
    }
}

void RenderingSystem::renderModelEntity(Entity entity, const RenderFrameContext& frameContext)
{
    auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);
    auto& modelRenderable = gCoordinator.GetComponent<ModelRenderable>(entity);
    auto const material = modelRenderable.material;

    if (modelRenderable.modelLoader && material.shader) {
        bindMaterial(material);

        glm::mat4 const modelMatrix = buildModelMatrix(transform, modelRenderable.visualOffsetPos);
        uploadCommonMatrices(material.shader, modelMatrix, frameContext.view, frameContext.projection);

        if (material.useModelLighting) {
            uploadLightingUniforms(material.shader, frameContext);
        }

        modelRenderable.modelLoader->draw(*material.shader);
    }
}

/* ----- Render helpers ----- */

void RenderingSystem::bindMaterial(const RenderMaterial& material) const
{
    if (!material.shader) return;

    material.shader->use();

    if (material.baseTexture) {
        // Explicitly bind base texture to slot 0 to match textured shader sampler layout.
        glActiveTexture(GL_TEXTURE0);
        material.baseTexture->bind();
        glUniform1i(
            glGetUniformLocation(*material.shader, "baseColorTexture"),
            0
        );
    }

    glm::vec2 const textureScrollOffset = material.useTextureScroll
        ? material.textureScrollOffset
        : glm::vec2(0.0f);

    glUniform2fv(
        glGetUniformLocation(*material.shader, "textureScrollOffset"),
        1, glm::value_ptr(textureScrollOffset)
    );
}

glm::mat4 RenderingSystem::buildModelMatrix(const PhysxTransform& transform, const glm::vec3& localOffset) const
{
    glm::vec3 const offsetInWorldSpace = transform.rot * localOffset;
    glm::vec3 const visualPos = transform.pos + offsetInWorldSpace;

    return glm::translate(glm::mat4(1.0f), visualPos)
        * glm::toMat4(transform.rot)
        * glm::scale(glm::mat4(1.0f), transform.scale);
}

void RenderingSystem::uploadCommonMatrices(const std::shared_ptr<ShaderProgram>& shader,
    const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const
{
    glUniformMatrix4fv(
        glGetUniformLocation(*shader, "model"),
        1, GL_FALSE, &model[0][0]
    );

    glUniformMatrix4fv(
        glGetUniformLocation(*shader, "view"),
        1, GL_FALSE, &view[0][0]
    );

    glUniformMatrix4fv(
        glGetUniformLocation(*shader, "projection"),
        1, GL_FALSE, &projection[0][0]
    );
}

void RenderingSystem::uploadLightingUniforms(
    const std::shared_ptr<ShaderProgram>& shader,
    const RenderFrameContext& frameContext) const
{
    // glGetUniformLocation may return -1 for shaders that optimize out optional fields
    // glUniform* then becomes a no-op (nothing happens)
    // This lets generic uploads work with shader versions without branching
    glUniform3fv(
        glGetUniformLocation(*shader, "lightPos"),
        1, glm::value_ptr(frameContext.lighting.lightPosition)
    );

    glUniform3fv(
        glGetUniformLocation(*shader, "lightColor"),
        1, glm::value_ptr(frameContext.lighting.lightColor)
    );

    glUniform3fv(
        glGetUniformLocation(*shader, "viewPos"),
        1, glm::value_ptr(frameContext.cameraPosition)
    );

    // Shadow-related uniforms
    glUniform3fv(
        glGetUniformLocation(*shader, "lightDir"),
        1, glm::value_ptr(frameContext.lighting.lightDirection)
    );

    glUniform1i(
        glGetUniformLocation(*shader, "shadowsEnabled"),
        frameContext.lighting.shadowsEnabled ? 1 : 0
    );

    glUniformMatrix4fv(
        glGetUniformLocation(*shader, "lightViewProjection"),
        1, GL_FALSE, glm::value_ptr(frameContext.lighting.lightViewProjection)
    );
}

void RenderingSystem::ensureShadowDepthResources(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return;
    }

    if (shadowDepthResources.framebuffer != 0
        && shadowDepthResources.depthTexture != 0
        && shadowDepthResources.width == width
        && shadowDepthResources.height == height) {
        return;
    }

    releaseShadowDepthResources();

    glGenFramebuffers(1, &shadowDepthResources.framebuffer);
    glGenTextures(1, &shadowDepthResources.depthTexture);

    glBindTexture(GL_TEXTURE_2D, shadowDepthResources.depthTexture);
    // Allocate depth-only texture storage.
    // nullptr means reserve GPU memory without CPU upload.
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT,
        width,
        height,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowDepthResources.framebuffer);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        shadowDepthResources.depthTexture,
        0
    );
    // This FBO is depth-only, so disable color writes/reads explicitly.
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        logger::error("Shadow depth framebuffer is incomplete");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        releaseShadowDepthResources();
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shadowDepthResources.width = width;
    shadowDepthResources.height = height;
}

void RenderingSystem::releaseShadowDepthResources()
{
    if (shadowDepthResources.depthTexture != 0) {
        glDeleteTextures(1, &shadowDepthResources.depthTexture);
        shadowDepthResources.depthTexture = 0;
    }

    if (shadowDepthResources.framebuffer != 0) {
        glDeleteFramebuffers(1, &shadowDepthResources.framebuffer);
        shadowDepthResources.framebuffer = 0;
    }

    shadowDepthResources.width = 0;
    shadowDepthResources.height = 0;
}

/* ----- Update step ----- */

void RenderingSystem::update(float deltaTime)
{
    if (freeCamera) {
        freeCamera->movementSpeed(camSpeed * 10.f);
    }
    if (racingCamera) {
        racingCamera->update(deltaTime);
    }

    processInput(deltaTime);

    updateSkyboxFollow();
    
    render();
}

void RenderingSystem::initVideo() {
    videoShader = std::make_shared<ShaderProgram>("assets/shaders/video.vert", "assets/shaders/video.frag");

    CPU_Geometry cpuQuad;
    cpuQuad.positions = { {-1,1,0}, {-1,-1,0}, {1,-1,0}, {1,1,0} };
    cpuQuad.uvs = { {0,1}, {0,0}, {1,0}, {1,1} };
    cpuQuad.indices = { 0, 1, 2, 0, 2, 3 };

    cpuQuad.colors.resize(4, glm::vec3(1.0f));
    cpuQuad.normals.resize(4, glm::vec3(0, 0, 1));

    videoQuadGPU.Update(cpuQuad);
    videoSystemsInitialized = true;
}

void RenderingSystem::drawFullscreenQuad(GLuint textureID) {
    if (!videoSystemsInitialized) initVideo();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    videoShader->use();

    GLuint progID = (GLuint)*videoShader;

    GLint texLoc = glGetUniformLocation(progID, "videoTexture");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glUniform1i(texLoc, 0);

    videoQuadGPU.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glEnable(GL_DEPTH_TEST);
}


void RenderingSystem::processInput(float deltaTime)
{
    // Handle camera toggle with F key
    if (inputManager->isKeyPressedOnce(GLFW_KEY_F)) {
        toggleCamera();
    }

    processCameraInput(deltaTime);
}

void RenderingSystem::processCameraInput(float deltaTime)
{
    auto const cursorPosition = inputManager->cursorPosition();

    // Check if we're using FreeCamera
    if (activeCamera == freeCamera.get())
    {
        // FreeCamera uses mouse movement when right mouse button is held
        if (inputManager->isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            if (cursorPositionIsSetOnce) {
                auto const deltaPosition = cursorPosition - previousCursorPosition;
                freeCamera->processMouseMovement(
                    static_cast<float>(deltaPosition.x),
                    static_cast<float>(-deltaPosition.y)  // Invert Y for natural feel
                );
            }
        }

        // IJKL/UO controls for FreeCamera movement (don't interfere with WASD game controls)
        if (inputManager->isKeyPressed(GLFW_KEY_I))
            freeCamera->processKeyboard(FreeCamera::Movement::FORWARD, deltaTime);
        if (inputManager->isKeyPressed(GLFW_KEY_K))
            freeCamera->processKeyboard(FreeCamera::Movement::BACKWARD, deltaTime);
        if (inputManager->isKeyPressed(GLFW_KEY_J))
            freeCamera->processKeyboard(FreeCamera::Movement::LEFT, deltaTime);
        if (inputManager->isKeyPressed(GLFW_KEY_L))
            freeCamera->processKeyboard(FreeCamera::Movement::RIGHT, deltaTime);
        if (inputManager->isKeyPressed(GLFW_KEY_U))
            freeCamera->processKeyboard(FreeCamera::Movement::UP, deltaTime);
        if (inputManager->isKeyPressed(GLFW_KEY_O))
            freeCamera->processKeyboard(FreeCamera::Movement::DOWN, deltaTime);
    }
    else if (activeCamera == turntableCamera.get())
    {
        // poll controller state first
        inputManager->pollControllerInputs();

        // TurnTableCamera uses right-click drag
        if (inputManager->isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            if (cursorPositionIsSetOnce) {
                float const aspectRatio = static_cast<float>(vWidth) / static_cast<float>(vHeight);
                auto const deltaPosition = cursorPosition - previousCursorPosition;
                turntableCamera->adjustTheta(-static_cast<float>(deltaPosition.x) * deltaTime * this->camSpeed * (1 / aspectRatio));
                turntableCamera->adjustPhi(-static_cast<float>(deltaPosition.y) * deltaTime * this->camSpeed);
            }
        }
        else if (inputManager->isControllerConnected())
        {
            float rx = inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_RIGHT_X);
            float ry = inputManager->getControllerAxis(GLFW_GAMEPAD_AXIS_RIGHT_Y);

            const float deadzone = 0.20f;
            if (std::abs(rx) < deadzone) rx = 0.0f;
            if (std::abs(ry) < deadzone) ry = 0.0f;

            if (rx != 0.0f || ry != 0.0f) {
                float const aspectRatio = static_cast<float>(vWidth) / static_cast<float>(vHeight);
                float sensitivity = 2.0f;
                turntableCamera->adjustTheta(-rx * deltaTime * this->camSpeed * sensitivity * (1.0f / aspectRatio));
                turntableCamera->adjustPhi(-ry * deltaTime * this->camSpeed * sensitivity);
            }
        }
    }

    // Always update cursor position tracking
    cursorPositionIsSetOnce = true;
    previousCursorPosition = cursorPosition;
}

void RenderingSystem::updateSkyboxFollow()
{
    for (auto const& entity : mEntities) {
        if (!gCoordinator.HasComponent<Renderable>(entity)) {
            continue;
        }

        auto& renderable = gCoordinator.GetComponent<Renderable>(entity);
        if (!renderable.isSkybox) {
            continue;
        }

        auto& transform = gCoordinator.GetComponent<PhysxTransform>(entity);
        transform.pos = activeCamera->position();
    }
}

void RenderingSystem::toggleCamera()
{
    if (activeCamera == freeCamera.get())
    {
        activeCamera = racingCamera.get();
        logger::info("Switched to RacingCamera");
    }
    else if (activeCamera == racingCamera.get())
    {
        activeCamera = turntableCamera.get();
        logger::info("Switched to TurnTableCamera (Orbit)");
    }
    else
    {
        freeCamera->position(activeCamera->position());
        activeCamera = freeCamera.get();
        logger::info("Switched to FreeCamera");
    }
}

void RenderingSystem::updateCameraTarget(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& velocity)
{
    if (targetTransform) {
        targetTransform->setPosition(position);
    }

    if (racingCamera) {
        racingCamera->updateTarget(position, forward, velocity);
    }
}

void RenderingSystem::onMouseWheelChange(double xOffset, double yOffset)
{
    (void)xOffset;
    float scroll = -static_cast<float>(yOffset) * this->camZoomSpeed * 0.016f;

    if (activeCamera == freeCamera.get()) {
        freeCamera->adjustFov(scroll);
    }
    else if (activeCamera == turntableCamera.get()) {
        turntableCamera->adjustDistance(scroll);
    }
}

void RenderingSystem::setSnowFrame(const SnowFrame& frame)
{
    snowRenderer.submitSnowFrame(frame);
}
