#ifndef CSOS_INTERFACES_SHT45_H_
#define CSOS_INTERFACES_SHT45_H_

#include <Arduino.h>
#include <I2CIP.h>

// MACROS

// Registers
// #define SHT45_MEAS_HIGHREP_STRETCH  (uint16_t)0x2C06  // Measurement High Repeatability with Clock Stretch Enabled
// #define SHT45_MEAS_MEDREP_STRETCH   (uint16_t)0x2C0D  // Measurement Medium Repeatability with Clock Stretch Enabled
// #define SHT45_MEAS_LOWREP_STRETCH   (uint16_t)0x2C10  // Measurement Low Repeatability with Clock Stretch Enabled
// #define SHT45_MEAS_HIGHREP          (uint16_t)0x2400  // Measurement High Repeatability with Clock Stretch Disabled
// #define SHT45_MEAS_MEDREP           (uint16_t)0x240B  // Measurement Medium Repeatability with Clock Stretch Disabled
// #define SHT45_MEAS_LOWREP           (uint16_t)0x2416  // Measurement Low Repeatability with Clock Stretch Disabled
// #define SHT45_READSTATUS            (uint16_t)0xF32D  // Read Out of Status Register
// #define SHT45_CLEARSTATUS           (uint16_t)0x3041  // Clear Status
// #define SHT45_SOFTRESET             (uint16_t)0x30A2  // Soft Reset
// #define SHT45_HEATEREN              (uint16_t)0x306D  // Heater Enable
// #define SHT45_HEATERDIS             (uint16_t)0x3066  // Heater Disable
// #define SHT45_REG_HEATER_BIT        (uint16_t)0x000d  // Status Register Heater Bit

// Settings
 // 0x44
#define I2CIP_SHT45_ADDRESS 68
#define SHT45_DELAY 10 // Write/Read Sensing Delay

typedef enum {
  SHT45_HEATER_DISABLE  = 0x00,
  SHT45_HEATER_ENABLE   = 0x01,
} args_sht45_t;

typedef struct {
  float temperature;
  float humidity;
} state_sht45_t;

// const char i2cip_sht45_id_progmem[] PROGMEM = {"SHT45"};

// Interface class for the SHT45 air temperature and humidity sensor
class SHT45 : public I2CIP::Device, public I2CIP::InputInterface<state_sht45_t, args_sht45_t> {
  I2CIP_DEVICE_CLASS_BUNDLE(SHT45);
  I2CIP_INPUT_USE_RESET(state_sht45_t, args_sht45_t);

  // I2CIP_INPUT_USE_TOSTRING(state_sht45_t, "{\"temperature\": %.1f, \"humidity\": %.1f}");
  // I2CIP_INPUT_ADD_PRINTCACHE(state_sht45_t, "Temperature: %.1f deg C, Humidity: %.1f \%");
  private:
    char print_buffer[I2CIP_INPUT_PRINTBUFFER_SIZE];
    char cache_buffer[I2CIP_INPUT_CACHEBUFFER_SIZE];
  public:
    const char* cacheToString(void) override;
    const char* printCache(void) override;
  private:

    // SHT45(i2cip_fqa_t fqa) : I2CIP::Device(fqa, i2cip_sht45_id_progmem, _id), I2CIP::InputInterface<state_sht45_t, args_sht45_t>((I2CIP::Device*)this) { }

    #ifdef MAIN_CLASS_NAME
    friend class MAIN_CLASS_NAME;
    #endif
  public:
    SHT45(i2cip_fqa_t fqa, const i2cip_id_t& id);
    
    i2cip_errorlevel_t get(state_sht45_t& value, const args_sht45_t& args) override;
};

#endif