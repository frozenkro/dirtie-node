#include "sensor.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdint.h>
#include <stdio.h>

// I2C address
static const uint8_t ADAFRUIT_SENSOR_ADDR = 0x36; // soil moisture/temp
static const uint8_t BME280_ADDR = 0x76;          // pressure/humidity
static const uint8_t LTR559_ADDR = 0x23;          // light/proximity sensor

// Registers
static const uint8_t REG_CAPACITANCE_BASE = 0x0F;
static const uint8_t REG_CAPACITANCE_FUNC = 0x10;
static const uint8_t REG_STATUS_BASE = 0x00;
static const uint8_t REG_STATUS_TEMP_FUNC = 0x04;

static const uint8_t REG_DEVICE_ID = 0x0;
static const uint8_t REG_BME280 = 0x76;

int seesaw_reg_read(i2c_inst_t *i2c, const uint addr, const uint8_t baseReg,
                    const uint8_t funcReg, uint8_t *buf, const uint8_t nbytes) {
  int num_bytes_read = 0;

  if (nbytes < 1) {
    return 0;
  }

  uint8_t regs[] = {baseReg, funcReg};
  i2c_write_blocking(i2c, addr, regs, 2, false);
  sleep_ms(250);
  num_bytes_read = i2c_read_blocking(i2c, addr, buf, nbytes, false);

  return num_bytes_read;
}

uint16_t seesaw_read_16(i2c_inst_t *i2c, const uint addr, const uint8_t baseReg,
                        const uint8_t funcReg) {
  uint8_t buf[2] = {0};

  int num_bytes_read = seesaw_reg_read(i2c, addr, baseReg, funcReg, buf, 2);
  if (num_bytes_read < 0) {
    printf("Error reading sensor. \nBase Register: %c\nFunction Register: %c",
           baseReg, funcReg);
    return -1;
  }

  uint16_t result = (uint16_t)buf[0] << 8 | buf[1];
  return result;
}

int sensor_test() {

  const uint sda_pin = 4;
  const uint scl_pin = 5;

  // ports
  i2c_inst_t *i2c = i2c0;

  // init I2C port @ 100 khz her adafruit seesaw recommendation
  i2c_init(i2c0, 100 * 1000);

  gpio_set_function(sda_pin, GPIO_FUNC_I2C);
  gpio_set_function(scl_pin, GPIO_FUNC_I2C);
  gpio_pull_up(sda_pin);
  gpio_pull_up(scl_pin);

  uint16_t capacitance = seesaw_read_16(
      i2c, ADAFRUIT_SENSOR_ADDR, REG_CAPACITANCE_BASE, REG_CAPACITANCE_FUNC);
  uint16_t temp = seesaw_read_16(i2c, ADAFRUIT_SENSOR_ADDR, REG_STATUS_BASE,
                                 REG_STATUS_TEMP_FUNC);

  printf("capacitance: %hu\n", capacitance);
  printf("temp: %hu\n", temp);

  return 0;
}
