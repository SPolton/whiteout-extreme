
// This file defines the collision flags used in the physics simulation.
// These are used to specify which types of objects should collide with each other.
enum Flags {
    // Collision flags for the different types of objects in the simulation
    COLLISION_FLAG_GROUND = 1 << 0,
    COLLISION_FLAG_WHEEL = 1 << 1,
    COLLISION_FLAG_CHASSIS = 1 << 2,
    COLLISION_FLAG_OBSTACLE = 1 << 3,
    COLLISION_FLAG_AVALANCHE = 1 << 4,

    // Collision flags for what each type of object should collide against
    COLLISION_FLAG_GROUND_AGAINST = COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_AVALANCHE,
    COLLISION_FLAG_CHASSIS_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE,
    COLLISION_FLAG_OBSTACLE_AGAINST = COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL | COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_AVALANCHE,
    COLLISION_FLAG_AVALANCHE_AGAINST = COLLISION_FLAG_OBSTACLE
};
