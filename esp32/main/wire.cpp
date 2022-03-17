#include <assert.h>

#include "wire.h"
#include "macro.h"


// List of diameters. Starts with awg 14, ends with awg 50
static float  awg_diam_mm[]= {
1.70053,1.51765,1.35636,1.21412,1.08585,0.97028,0.86995,0.77724,0.69469,0.62484,
0.56007,0.50038,0.44831,0.40386,0.36195,0.32512,0.29083,0.26162,0.23749,0.21209,
0.18796,0.16764,0.15113,0.13589,0.12065,0.10541,0.09398,0.08509,0.0762,0.06731,
0.06096,0.053213,0.048387,0.043815,0.038989,0.035179,0.032385
};

#define FIRST_AWG 14
#define LAST_AWG (ARRAY_COUNT(awg_diam_mm)+14-1)

float awg_to_mm(int awg)
{
    assert (awg >= FIRST_AWG && awg <= LAST_AWG);
    return awg_diam_mm[awg-FIRST_AWG];
}
