#ifndef MACRO_H_
#define MACRO_H_

#define CLAMP(x,min,max) (x<min?min:(x>max?max:x))
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

#endif // MACRO_H_
