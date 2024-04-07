#include "sensor.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/time.h"
#include <stdio.h>

#define DEBUG_printf printf

// I2C address
static const uint8_t ADAFRUIT_SENSOR_ADDR = 0x36;

// Registers 
static const uint8_t REG_CAPACITANCE_BASE = 0x0F;
static const uint8_t REG_CAPACITANCE_FUNC = 0x10;

int reg_read(i2c_inst_t *i2c,
            const uint addr,
            const uint8_t baseReg,
            const uint8_t funcReg,
            uint8_t *buf,
            const uint8_t nbytes) {
  int num_bytes_read = 0;

  if (nbytes < 1) {
    return 0;
  }

  i2c_write_blocking(i2c, addr, &baseReg, 1, true);
  i2c_write_blocking(i2c, addr, &funcReg, 1, true);
  sleep_ms(1000);
  num_bytes_read = i2c_read_blocking(i2c, addr, buf, nbytes, false);

  return num_bytes_read;
}

int test_sensor() {

  const uint sda_pin = 4;
  const uint scl_pin = 5;

  // ports
  i2c_inst_t *i2c = i2c0;

  // buffer for storing reads
  uint8_t data[6];

  // init I2C port @ 100 khz her adafruit seesaw recommendation
  i2c_init(i2c, 10000);

  gpio_set_function(sda_pin, GPIO_FUNC_I2C);
  gpio_set_function(scl_pin, GPIO_FUNC_I2C);
  
  int bytes_read;
  bytes_read = reg_read(i2c, ADAFRUIT_SENSOR_ADDR, REG_CAPACITANCE_BASE, REG_CAPACITANCE_FUNC, data, 4);

  DEBUG_printf("bytes read: %i", bytes_read);

  return 0;
}
