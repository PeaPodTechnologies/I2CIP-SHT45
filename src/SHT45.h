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
#define SHT45_ADDRESS 0x44 // Default
#define SHT45_DELAY 10 // Write/Read Sensing Delay

using namespace I2CIP;

typedef enum {
  SHT45_HEATER_DISABLE  = 0x00,
  SHT45_HEATER_ENABLE   = 0x01,
} args_sht45_t;

typedef struct {
  float temperature;
  float humidity;
} state_sht45_t;

const char wiipod_sht45_id_progmem[] PROGMEM = {"SHT45"};

// Interface class for the SHT45 air temperature and humidity sensor
class SHT45 : public Device, public InputInterface<state_sht45_t, args_sht45_t> {
  private:
    static bool _id_set;
    static char _id[];

    // Note: unsigned 16-bit args are TRUNCATED to 12-bit PWM control
    const state_sht45_t default_cache = { NAN, NAN };
    const args_sht45_t default_a = SHT45_HEATER_DISABLE;

    SHT45(const i2cip_fqa_t& fqa);

    static void loadID(void);

    #ifdef MAIN_CLASS_NAME
    friend class MAIN_CLASS_NAME;
    #endif
  public:
    SHT45(const i2cip_fqa_t& fqa, const i2cip_id_t& id);

    static Device* sht45Factory(const i2cip_fqa_t& fqa, const i2cip_id_t& id);
    static Device* sht45Factory(const i2cip_fqa_t& fqa);
    
    i2cip_errorlevel_t get(state_sht45_t& value, const args_sht45_t& args) override;

    const args_sht45_t& getDefaultA(void) const override;
    void clearCache(void) override;

    static const char* getStaticIDBuffer() { return SHT45::_id_set ? SHT45::_id : nullptr; }
};

#endif