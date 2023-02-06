#include "Utils.h"
#include "GlobalParams.h"

double getCurrentTimeStamp() {
    return (double)sc_time_stamp().to_double() / GlobalParams::clock_period_ps;
}