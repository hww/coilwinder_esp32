#include "mathlib.h"
#include <math.h>

const float EPSILON = 0.000001;

bool inexact_compare(float x, float y) {
  return ((fabs(x) - fabs(y)) < EPSILON);
}
int get_inexact_direction(float v) {
  return inexact_compare(v,0) ? 0 : (v > 0 ? 1 : -1);
}
int get_direction(float v) {
  return v == 0 ? 0 : (v > 0 ? 1 : -1);
}
int get_direction(int v) {
  return v == 0 ? 0 : (v > 0 ? 1 : -1);
}

float clamp(float v, float min, float max) {
    return (v<min) ? min : (v>max ? max : v);
}

float clamp01(float v) {
    return (v<0) ? 0 : (v>1.0f ? 1.0f : v);
}

float lerp(float v, float min, float max) {
    return ((max-min) * v) + min;
}

int clamp(int v, int min, int max) {
    return (v<min) ? min : (v>max ? max : v);
}

float get_normalized_position(float v, float min, float max) {
    return clamp01((v-min) / (max-min));
}
