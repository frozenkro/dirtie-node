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
  //i2c_write_blocking(i2c, addr, &baseReg, 1, true);
  i2c_write_blocking(i2c, addr, 0x00, 1, true);
  sleep_ms(1000);
  num_bytes_read = i2c_read_blocking(i2c, addr, buf, 4, false);
  
  uint8_t tempRegs[] = { REG_STATUS_BASE, REG_STATUS_TEMP_FUNC };
  i2c_write_blocking(i2c, addr, tempRegs, 2, true);
  i2c_write_blocking(i2c, addr, &REG_STATUS_TEMP_FUNC, 1, true);
  sleep_ms(250);
  num_bytes_read += i2c_read_blocking(i2c, addr, &buf[2], 2, false);

  return num_bytes_read;
}
int reg_read_enviro(i2c_inst_t *i2c,
            const uint addr,
            const uint8_t reg,
            int32_t* pressure,
            int32_t* temp) {

  int num_bytes_read = 0;

  // (copied this comment from bmp example, registers are the same for BME)
  // BMP280 data registers are auto-incrementing and we have 3 temperature and
  // pressure registers each, so we start at 0xF7 and read 6 bytes to 0xFC
  // note: normal mode does not require further ctrl_meas and config register writes

  uint8_t buf[6];
  i2c_write_blocking(i2c, addr, &reg, 1, true);  // true to keep master control of bus
  i2c_write_blocking(i2c, addr, &reg, 1, true);  // true to keep master control of bus
  sleep_ms(2500);
  num_bytes_read = i2c_read_blocking(i2c, addr, buf, 6, false);  // false - finished with bus

    // store the 20 bit read in a 32 bit signed integer for conversion
    *pressure = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    *temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);

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
  bytes_read = reg_read(i2c, ADAFRUIT_SENSOR_ADDR, REG_CAPACITANCE_BASE, REG_CAPACITANCE_FUNC, data, 4);

  uint16_t capacitance = (uint16_t)data[0] << 8 | data[1];
  uint16_t temp = (uint16_t)data[2] << 8 | data[3];

  printf("bytes read: %d\n", bytes_read);
  printf("capacitance: %hu\n", capacitance);
  printf("temp: %hu\n", temp);

  int bme_bytes_read;

  int32_t bme_pressure;
  int32_t bme_temperature;
  bme_bytes_read = reg_read_enviro(i2c0, BME280_ADDR, 0xF7, &bme_pressure, &bme_temperature);
  printf("bytes read from bme: %d\n", bme_bytes_read);
  printf("bme pressure %hu\n", bme_pressure);
  printf("bme temperature %hu\n", bme_temperature);

  return 0;
}
