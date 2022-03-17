#ifndef TIME_H_
#define TIME_H_

#include <stdint.h>

class Time {
 public:

  Time();

  void update();

  float time;
  float delta_time;
  float speed;

private:
  uint64_t last_time;

};


#endif // TIME_H_
