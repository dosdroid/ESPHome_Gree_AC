#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/gpio.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "ir_Gree.h"

namespace esphome {
namespace gree {

class GreeClimate : public climate::Climate, public Component {
private:
  IRGreeAC* transmitter_;
  sensor::Sensor *temperature_sensor_{nullptr};

  void setClimateMode(const climate::ClimateMode climateMode);
  void setTargetTemperature(const float targetTemperature);
  void setFanMode(const climate::ClimateFanMode fanMode);
  //void setSwingMode(const climate::ClimateSwingMode swingMode);

public:
  GreeClimate(InternalGPIOPin *pin);

  void setup() override;
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;
  void set_temperature_sensor(sensor::Sensor *sensor) { this->temperature_sensor_ = sensor; }
};

} // namespace gree
} // namespace esphome
