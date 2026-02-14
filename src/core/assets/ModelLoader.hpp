#pragma once

#include "Model.hpp"
#include "core/render/ShaderProgram.hpp"
#include "utils/logger.h"

#include <memory>
#include <string>
#include <stdexcept>

class ModelLoader {
public:
    explicit ModelLoader(const std::string& path, bool gammaCorrection = false)
        : model(std::make_unique<Model>(path, gammaCorrection))
    {
        if (model->meshes.empty()) {
            logger::error("Failed to load model from: {}", path);
            throw std::runtime_error("Failed to load model from: " + path);
        }
    }

    void draw(ShaderProgram& shader) {
        if (model) {
            model->draw(shader);
        }
    }

    size_t getMeshCount() const {
        return model ? model->meshes.size() : 0;
    }

    const std::string& getDirectory() const {
        return model->directory;
    }

    const std::string& getPath() const {
        return model->path_;
    }

private:
    std::unique_ptr<Model> model;
};
