#pragma once

#include "Texture.hpp"
#include "core/buffer/Geometry.hpp"
#include "core/render/ShaderProgram.hpp"
#include <memory>
#include <string>
#include <unordered_map>

// Asset manager for centralized resource loading and sharing
// Uses shared_ptr to allow multiple entities to share the same resources
class AssetManager {
public:
    static AssetManager& getInstance() {
        static AssetManager instance;
        return instance;
    }

    // Load or get cached texture
    std::shared_ptr<Texture>
    loadTexture(const std::string& path, GLint interpolation = GL_LINEAR, GLint wrapMode = GL_REPEAT);

    // Load or get cached shader (convention-based: assumes shaders/{name}.vert and shaders/{name}.frag)
    std::shared_ptr<ShaderProgram>
    loadShader(const std::string& name);

    // Load or get cached shader
    std::shared_ptr<ShaderProgram>
    loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath);

    // Create or get cached geometry
    std::shared_ptr<GPU_Geometry>
    loadGeometry(const std::string& name, const CPU_Geometry& cpuData);

    // Get cached CPU geometry data
    std::shared_ptr<CPU_Geometry>
    getCPUGeometry(const std::string& name) const;

    // Clear all cached resources
    void clearAll();

    // Clear specific resource types
    void clearTextures();
    void clearShaders();
    void clearGeometry();

private:
    AssetManager() = default;
    ~AssetManager() = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Base path constants
    const std::string assetsFolder = "assets";
    const std::string texturesFolder = "textures";
    const std::string shadersFolder = "shaders";

    std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;
    std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shaderCache;
    std::unordered_map<std::string, std::shared_ptr<GPU_Geometry>> geometryCache;
    std::unordered_map<std::string, std::shared_ptr<CPU_Geometry>> cpuGeometryCache;
};
