/* 
* Reading Battery Level
* Found in - https://forum.seeedstudio.com/t/xiao-ble-sense-battery-level-and-charging-status/263248/24
* Edited by: Gerbert Funes 
* 
* This is reading the analog pin V_BAT to get access of pin P0.14 battery voltage value. Assigns that voltage
* to a percentage range and sets LED colors to let the user know at what charge the battery is at.
*
* Note: Leave pin P0.14 in LOW position. The "HackAnalogIn" class sets up and handles the concern that P0.31 
* may burn out when charging a battery. This class also stops the bug where when reading out of P0.14 turns 
* off the IMU and BLE capabilities. 
*
*/

#include <nrf52840.h>
#include <nrfx_saadc.h>
#include <AnalogIn.h>
#include <pinDefinitions.h>

class HackAnalogIn: public mbed::AnalogIn {
  using mbed::AnalogIn::AnalogIn;
  public:
    analogin_t getAnalogIn_t();
};

analogin_t HackAnalogIn::getAnalogIn_t() {
  return this->_adc;
}

void startReadingBatteryLevel(nrf_saadc_value_t* buffer) {
  auto pin = PIN_VBAT;
  PinName name = analogPinToPinName(pin);
  if (name == NC){
    return;
  }

  HackAnalogIn* adc = static_cast<HackAnalogIn*>(analogPinToAdcObj(pin));
  if (adc == NULL){
    adc = new HackAnalogIn(name);
    analogPinToAdcObj(pin) = static_cast<mbed::AnalogIn*>(adc);
#ifdef ANALOG_CONFIG
    if (isAdcConfigChanged){
      adc->configure(adcCurrentConfig);
    }
#endif
  }

  nrfx_saadc_buffer_convert(buffer, 1);
  nrfx_err_t ret = nrfx_saadc_sample();
  if (ret == NRFX_ERROR_BUSY){
    // failed to start sampling
    return;
  }
}

nrf_saadc_value_t BatteryLevel = { 0 };

float vBat = 0.0;
float batPercent = 0.0;

void monitor_battery_level(void) {
  // Monitor the Battery Level
  static unsigned long _lastT = 0;
  unsigned long _t = millis();

  if (_t - _lastT > 1000){
    // read battery level every 1 second
    startReadingBatteryLevel(&BatteryLevel);
    _lastT = _t;
  }

  // check if ADC conversion has completed
  if (nrf_saadc_event_check(NRF_SAADC_EVENT_DONE)){
    // ADC conversion completed. Reading is stored in BatteryLevel
    nrf_saadc_event_clear(NRF_SAADC_EVENT_DONE);
     vBat = ((float)BatteryLevel / 4096 * 3.3 / 510 * (1000 + 510)); 

     batPercent = 100 - (((4.0 - vBat)/vBat)*100);

    if (batPercent > 90){
      pinMode(LEDG, OUTPUT);
      pinMode(LEDR, OUTPUT);
      pinMode(LEDB, OUTPUT);
      digitalWrite(LEDG,LOW); 
      digitalWrite(LEDR,HIGH);
      digitalWrite(LEDB,HIGH); 

      pinMode (P0_13, OUTPUT);
      digitalWrite(P0_13, LOW); // The battery charging current is selectable as 50mA or 100mA -> HIGH = 50mA, LOW = 100mA

    }
    else if(40 < batPercent < 90){
      pinMode(LEDG, OUTPUT);
      pinMode(LEDR, OUTPUT);
      pinMode(LEDB, OUTPUT);
      digitalWrite(LEDG,HIGH); 
      digitalWrite(LEDR,HIGH);
      digitalWrite(LEDB,LOW); 

      pinMode (P0_13, OUTPUT);
      digitalWrite(P0_13, LOW); // The battery charging current is selectable as 50mA or 100mA -> HIGH = 50mA, LOW = 100mA
    }
    else if(batPercent <= 40){
      pinMode(LEDG, OUTPUT);
      pinMode(LEDR, OUTPUT);
      pinMode(LEDB, OUTPUT);
      digitalWrite(LEDG,HIGH); 
      digitalWrite(LEDR,LOW);
      digitalWrite(LEDB,HIGH); 

      pinMode (P0_13, OUTPUT);
      digitalWrite(P0_13, LOW); // The battery charging current is selectable as 50mA or 100mA -> HIGH = 50mA, LOW = 100mA
    }
  }
}
void setupBatteryLevel() {

  // Battery Level setup
    pinMode(P0_14, OUTPUT);
    digitalWrite(P0_14,LOW);
  
  while(vBat == 0.0){
    monitor_battery_level();
  }
}