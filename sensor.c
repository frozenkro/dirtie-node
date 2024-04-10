#include "sensor.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/time.h"
#include <stdio.h>

#define DEBUG_printf printf

// I2C address
static const uint8_t ADAFRUIT_SENSOR_ADDR = 0x36;//soil moisture/temp
static const uint8_t BME280_ADDR = 0x76;// pressure/humidity
static const uint8_t LTR559_ADDR = 0x23; // light/proximity sensor

// Registers 
static const uint8_t REG_CAPACITANCE_BASE = 0x0F;
static const uint8_t REG_CAPACITANCE_FUNC = 0x10;
static const uint8_t REG_STATUS_BASE = 0x00;
static const uint8_t REG_STATUS_TEMP_FUNC = 0x04;

static const uint8_t REG_DEVICE_ID = 0x0;
static const uint8_t REG_BME280 = 0x76;

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

  uint8_t regs[] = { baseReg, funcReg };
  i2c_write_blocking(i2c, addr, regs, 2, true);
  sleep_ms(1000);
  num_bytes_read = i2c_read_blocking(i2c, addr, buf, 2, false);
  
  i2c_write_blocking(i2c, addr, &REG_STATUS_BASE, 1, true);
  i2c_write_blocking(i2c, addr, &REG_STATUS_TEMP_FUNC, 1, true);
  sleep_ms(250);
  num_bytes_read += i2c_read_blocking(i2c, addr, &buf[2], 2, false);

  return num_bytes_read;
}
int reg_read_enviro(i2c_inst_t *i2c,
            const uint addr,
            const uint8_t reg,
            uint8_t *buf,
            const uint8_t nbytes) {

  int num_bytes_read = 0;

  if (nbytes < 1) {
    return 0;
  }

  i2c_write_blocking(i2c, addr, &reg, 1, true);

  sleep_ms(1000);
  num_bytes_read = i2c_read_blocking(i2c, addr, buf, nbytes, false);

  return num_bytes_read;
}
int test_sensor() {

  const uint sda_pin = 4;
  const uint scl_pin = 5;

  // ports
  i2c_inst_t *i2c = i2c0;
  //
  // buffer for storing reads
  uint8_t data[4] = {0};

  // init I2C port @ 100 khz her adafruit seesaw recommendation
  i2c_init(i2c0, 100 * 1000);

  gpio_set_function(sda_pin, GPIO_FUNC_I2C);
  gpio_set_function(scl_pin, GPIO_FUNC_I2C);
  gpio_pull_up(sda_pin);
  gpio_pull_up(scl_pin);

  
  int bytes_read;
  //bytes_read = reg_read_enviro(i2c0, BME280_ADDR, REG_BME280, data, 1);
  bytes_read = reg_read(i2c, ADAFRUIT_SENSOR_ADDR, REG_CAPACITANCE_BASE, REG_CAPACITANCE_FUNC, data, 4);

  uint16_t capacitance = (uint16_t)data[0] << 8 | data[1];
  uint16_t temp = (uint16_t)data[2] << 8 | data[3];

  printf("bytes read: %d", bytes_read);
  printf("capacitance: %hu", capacitance);
  printf("temp: %hu", temp);

  return 0;
}
