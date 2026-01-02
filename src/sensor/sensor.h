#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

int sensor_init();
int sensor_check(uint16_t *capacitance, uint16_t *temperature);

int sensor_test();

#endif
