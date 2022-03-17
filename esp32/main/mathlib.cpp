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
