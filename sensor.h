#ifndef SENSOR_H
#define SENSOR_H

#include "hardware/i2c.h"

int test_sensor();

int reg_read(i2c_inst_t *i2c,
                const uint addr,
                const uint8_t baseReg,
                const uint8_t funcReg,
                uint8_t *buf,
                const uint8_t nbytes);
#endif
