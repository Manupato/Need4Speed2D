// car_design.h
#ifndef CAR_DESIGN_H
#define CAR_DESIGN_H

#include <cstdint>

// 0 .. 100
struct CarDesignStats {
    float speed;
    float engine_force;
    float handling;
    float weight;
    float shield;
};

struct CarDesignDef {
    CarDesignStats stats;
    float baseLength;
    float baseWidth;
};

#endif  // CAR_DESIGN_H
