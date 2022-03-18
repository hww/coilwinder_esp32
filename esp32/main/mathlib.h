#ifndef MATHLIB_H_
#define MATHLIB_H_

#include "typeslib.h"

#define CLAMP(x,min,max) (x<min?min:(x>max?max:x))

inline int sign(int v) { return v>=0 ? 1 : 0; }
inline float sign(float v) { return v>=0 ? 1 : 0; }

bool inexact_compare(float x, float y);
int get_inexact_direction(float v);

int get_direction(float v);
int get_direction(int v);

#endif // MATH_LIB_H_
