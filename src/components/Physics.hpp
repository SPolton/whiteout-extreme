#pragma once

#include <memory>

// Some complex PhysX-based objects store their data in class.
// The hybrid ECS component is a wrapper around the instance.

// Forward declaration
class Avalanche;

// Generic component that is still type-safe
template<typename T>
struct PhysicsComponent {
    std::shared_ptr<T> instance = nullptr;
};

struct AvalancheComponent {
    std::shared_ptr<Avalanche> instance = nullptr;
};
