#pragma once

#include <string>
#include <vector>
#include "Transform.h"
#include "Model.h"

enum class PhysType {
    None,
    StaticMesh,
    Vehicle,
    Trailer,
    RigidBody
};

enum class DrawType {
    Mesh,
    Decal,
    Invisible
};

// Entity Class
// Contains individual object information
class Entity {
public:
    std::string name = "unnamed_entity";
    PhysType physType = PhysType::None;
    DrawType drawType = DrawType::Mesh;
    PhysxTransform* transform;
    std::vector<PhysxTransform*> localTransforms;
    Model* model;
    //PlayerProperties* playerProperties;
    int nbChildEntities;
};
