#include "checkpoint_sensor.h"

CheckpointSensor::CheckpointSensor(b2ShapeId shapeId, int* orderPtr, int x, int y, bool goal):
        shapeId(shapeId), orderPtr(orderPtr), x(x), y(y), goal(goal) {}

b2ShapeId CheckpointSensor::get_shapeId() const { return shapeId; }

const int* CheckpointSensor::get_orderPtr() const { return orderPtr; }

int CheckpointSensor::get_position_in_px_X() const { return static_cast<int>((x + 0.5) * PPM); }
int CheckpointSensor::get_position_in_px_Y() const { return static_cast<int>((y + 0.5) * PPM); }

bool CheckpointSensor::is_goal() const { return goal; }
