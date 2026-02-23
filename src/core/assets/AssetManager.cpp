#include "AssetManager.hpp"
#include "AssetPath.hpp"
#include "utils/logger.h"

std::shared_ptr<Texture>
AssetManager::loadTexture(const std::string& path, GLint interpolation)
{
    // Check if texture is already cached
    auto it = textureCache.find(path);
    if (it != textureCache.end()) {
        logger::info("Using cached texture: {}", path);
        return it->second;
    }

    // Load new texture
    try {
        auto texture = std::make_shared<Texture>(path, interpolation);
        textureCache[path] = texture;
        logger::info("Loaded new texture: {}", path);
        return texture;
    }
    catch (const std::exception& e) {
        logger::error("Failed to load texture {}: {}", path, e.what());
        throw;
    }
}

std::shared_ptr<ShaderProgram>
AssetManager::loadShader(const std::string& name)
{
    // Convention-based path: assets/shaders/{name}.vert and assets/shaders/{name}.frag
    std::string vertPath = assetsFolder + "/" + shadersFolder + "/" + name + ".vert";
    std::string fragPath = assetsFolder + "/" + shadersFolder + "/" + name + ".frag";
    return loadShader(name, vertPath, fragPath);
}

std::shared_ptr<ShaderProgram>
AssetManager::loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath)
{
    // Check if shader is already cached
    auto it = shaderCache.find(name);
    if (it != shaderCache.end()) {
        logger::info("Using cached shader: {}", name);
        return it->second;
    }

    // Load new shader
    try {
        auto shader = std::make_shared<ShaderProgram>(vertPath, fragPath);
        shaderCache[name] = shader;
        logger::info("Loaded new shader: {}", name);
        return shader;
    }
    catch (const std::exception& e) {
        logger::error("Failed to load shader {}: {}", name, e.what());
        throw;
    }
}

std::shared_ptr<GPU_Geometry>
AssetManager::loadGeometry(const std::string& name, const CPU_Geometry& cpuData)
{
    // Check if geometry is already cached
    auto it = geometryCache.find(name);
    if (it != geometryCache.end()) {
        logger::info("Using cached geometry: {}", name);
        return it->second;
    }

    // Create new geometry
    auto gpuGeometry = std::make_shared<GPU_Geometry>();
    gpuGeometry->Update(cpuData);
    
    // Cache both GPU and CPU data
    geometryCache[name] = gpuGeometry;
    cpuGeometryCache[name] = std::make_shared<CPU_Geometry>(cpuData);
    
    logger::info("Created new geometry: {}", name);
    return gpuGeometry;
}

std::shared_ptr<CPU_Geometry>
AssetManager::getCPUGeometry(const std::string& name) const
{
    auto it = cpuGeometryCache.find(name);
    if (it != cpuGeometryCache.end()) {
        return it->second;
    }
    return nullptr;
}

void AssetManager::clearAll()
{
    clearTextures();
    clearShaders();
    clearGeometry();
    logger::info("Cleared all asset caches");
}

void AssetManager::clearTextures()
{
    textureCache.clear();
    logger::info("Cleared texture cache");
}

void AssetManager::clearShaders()
{
    shaderCache.clear();
    logger::info("Cleared shader cache");
}

void AssetManager::clearGeometry()
{
    geometryCache.clear();
    cpuGeometryCache.clear();
    logger::info("Cleared geometry cache");
}
