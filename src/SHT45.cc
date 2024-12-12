#include <sht45.h>

bool SHT45::_id_set = false;
char SHT45::_id[I2CIP_ID_SIZE];

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

SHT45::SHT45(const i2cip_fqa_t& fqa, const i2cip_id_t& id) : Device(fqa, id), InputInterface<state_sht45_t, args_sht45_t>((Device*)this) { }

SHT45::SHT45(const i2cip_fqa_t& fqa) : SHT45(fqa, getStaticIDBuffer()) { }

// Handles ID pointer assignment too
// NEVER returns nullptr, unless out of memory
Device* SHT45::sht45Factory(const i2cip_fqa_t& fqa) { return sht45Factory(fqa, getStaticIDBuffer()); }
Device* SHT45::sht45Factory(const i2cip_fqa_t& fqa, const i2cip_id_t& id) {
  if(!_id_set || id == nullptr) {
    loadID();
    return (Device*)(new SHT45(fqa, _id));
  }

  return (Device*)(new SHT45(fqa, id == nullptr ? _id : id));
}

void SHT45::loadID() {
  uint8_t idlen = strlen_P(wiipod_sht45_id_progmem);

  // Read in PROGMEM
  for (uint8_t k = 0; k < idlen; k++) {
    char c = pgm_read_byte_near(wiipod_sht45_id_progmem + k);
    SHT45::_id[k] = c;
  }

  SHT45::_id[idlen] = '\0';
  SHT45::_id_set = true;

  #ifdef I2CIP_DEBUG_SERIAL
    DEBUG_DELAY();
    I2CIP_DEBUG_SERIAL.print(F("SHT45 ID Loaded: '"));
    I2CIP_DEBUG_SERIAL.print(SHT45::_id);
    I2CIP_DEBUG_SERIAL.print(F("' @"));
    I2CIP_DEBUG_SERIAL.println((uintptr_t)(&SHT45::_id[0]), HEX);
    DEBUG_DELAY();
  #endif
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

const args_sht45_t& SHT45::getDefaultA(void) const {
  return this->default_a;
}

void SHT45::clearCache(void) {
  this->setCache(this->default_cache);
}