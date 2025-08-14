#include "SHT45.h"

#ifndef I2CIP_H_
#error "I2CIP must be in I2CIP-SHT45/libs, or, adjacent to I2CIP-SHT45 together in $PWD/libs"
#else

// #include <debug.h>

using namespace I2CIP;

#define SHT45_DEFAULT_CACHE { NAN, NAN }

I2CIP_DEVICE_INIT_STATIC_ID(SHT45);
I2CIP_INPUT_INIT_RESET(SHT45, state_sht45_t, SHT45_DEFAULT_CACHE, args_sht45_t, SHT45_HEATER_DISABLE);

void SHT45::parseJSONArgs(I2CIP::i2cip_args_io_t& argsDest, JsonVariant argsA, JsonVariant argsS, JsonVariant argsB) {
  if(argsA.is<bool>() || argsA.is<int>()) {
    argsDest.a = new args_sht45_t(argsA.as<int>() > 0 ? SHT45_HEATER_ENABLE : SHT45_HEATER_DISABLE);
  }
}

void SHT45::deleteArgs(I2CIP::i2cip_args_io_t& args) {
  delete (args_sht45_t*)args.a;
}

SHT45::SHT45(i2cip_fqa_t fqa, const i2cip_id_t& id) : I2CIP::Device(fqa, id), I2CIP::InputInterface<state_sht45_t, args_sht45_t>((I2CIP::Device*)this) { }

const char* SHT45::cacheToString(void) {
  memset(this->cache_buffer, 0, I2CIP_INPUT_CACHEBUFFER_SIZE);
  state_sht45_t value = this->getCache();
  snprintf(this->cache_buffer, I2CIP_INPUT_CACHEBUFFER_SIZE, "{\"temperature\": %.1f, \"humidity\": %.1f}", value.temperature, value.humidity);
  return this->cache_buffer;
}

const char* SHT45::printCache(void) {
  memset(this->print_buffer, 0, I2CIP_INPUT_PRINTBUFFER_SIZE);
  state_sht45_t value = this->getCache();
  snprintf(this->print_buffer, I2CIP_INPUT_PRINTBUFFER_SIZE, "Temperature: %.1f deg C, Humidity: %.1f \%", value.temperature, value.humidity);
  return this->print_buffer;
}

static uint8_t crc8(const uint8_t *data, int len) {
  /*
   *
   * CRC-8 formula from page 14 of SHT spec pdf
   *
   * Test data 0xBE, 0xEF should yield 0x92
   *
   * Initialization data 0xFF
   * Polynomial 0x31 (x8 + x5 +x4 +1)
   * Final XOR 0x00
   */

  const uint8_t POLYNOMIAL(0x31);
  uint8_t crc(0xFF);

  for (int j = len; j; --j) {
    crc ^= *data++;

    for (int i = 8; i; --i) {
      crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
    }
  }
  return crc;
}

i2cip_errorlevel_t SHT45::get(state_sht45_t& value, const args_sht45_t& args) {
  size_t buflen = 6;
  uint8_t buffer[buflen];

  // #ifdef I2CIP_DEBUG_SERIAL
  //   DEBUG_DELAY();
  //   I2CIP_DEBUG_SERIAL.print(F("[SHT45 | GET]"));
  //   DEBUG_DELAY();
  // #endif

  // Read registers
  // i2cip_errorlevel_t errlev = this->readRegister(0xFD, buffer, buflen, false); // No heater, high precision
  i2cip_errorlevel_t errlev = this->writeByte(0xFD, false); // No heater, high precision; DOESNT SET BUS
  I2CIP_ERR_BREAK(errlev);

  delay(SHT45_DELAY); // Needed for reading

  errlev = this->read(buffer, buflen, false, true, false); // No null-terminator; no bus set; resets bus
  I2CIP_ERR_BREAK(errlev);

  if (buflen != sizeof(buffer)) {
    return I2CIP_ERR_SOFT;
  }

  // Checksum
  // if (buffer[2] != crc8(buffer, 2) || buffer[5] != crc8(buffer + 3, 2)) {
  //   return I2CIP_ERR_SOFT;
  // }
  if (buffer[2] != crc8(buffer, 2) || buffer[5] != crc8(buffer + 3, 2)) return I2CIP_ERR_SOFT;

  // // Temperature conversion
  // int32_t stemp = (int32_t)(((uint32_t)buffer[0] << 8) | buffer[1]);
  // stemp = ((4375 * stemp) >> 14) - 4500;
  // value.temperature = (float)stemp / 100.0f;

  // // Humidity conversion
  // uint32_t shum = ((uint32_t)buffer[3] << 8) | buffer[4];
  // shum = (625 * shum) >> 12;
  // value.humidity = (float)shum / 100.0f;

  value.temperature = -45 + 175 * (((uint16_t)buffer[0] * 256 + (uint16_t)buffer[1]) / 65535.0f);
  value.humidity = min(max(-6 + 125 * (((uint16_t)buffer[3] * 256 + (uint16_t)buffer[4]) / 65535.0f), 0.0f), 100.0f);

  // #ifdef I2CIP_DEBUG_SERIAL
  //   DEBUG_DELAY();
  //   I2CIP_DEBUG_SERIAL.print(F("!!! "));
  //   I2CIP_DEBUG_SERIAL.print(value.temperature);
  //   I2CIP_DEBUG_SERIAL.print(F("C, "));
  //   I2CIP_DEBUG_SERIAL.print(value.humidity);
  //   I2CIP_DEBUG_SERIAL.print(F("% !!!"));
  //   DEBUG_DELAY();
  // #endif

  return I2CIP_ERR_NONE;
}

#endif