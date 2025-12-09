#ifndef CHECKPOINT_SENSOR_H
#define CHECKPOINT_SENSOR_H

#include <box2d/box2d.h>

class CheckpointSensor {
private:
    // shape real en el mundo
    b2ShapeId shapeId;
    // puntero estable (lista) al orden del checkpoint
    int* orderPtr;

    // Celdas de la matriz
    int x;
    int y;

    bool goal;

    static constexpr float PPM = 16.0f;  // pixels per meter

public:
    // Recive el id y el puntero a su orden al momento de crearse
    CheckpointSensor(b2ShapeId shapeId, int* orderPtr, int x, int y, bool goal);

    b2ShapeId get_shapeId() const;
    const int* get_orderPtr() const;

    // Si el sensor es meta
    bool is_goal() const;

    // Le devuelve ya la posicion en pixeles para el cliente
    int get_position_in_px_X() const;
    int get_position_in_px_Y() const;
};

#endif  // CHECKPOINT_SENSOR_H
