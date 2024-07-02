#include "gree.h"
#include "esphome/core/log.h"

static const char *const TAG = "gree.climate";

namespace esphome {
namespace gree {

GreeClimate::GreeClimate(InternalGPIOPin *pin)
{
  this->transmitter_ = new IRGreeAC(pin->get_pin());
}

void GreeClimate::setup()
{
  // Initialize provided temperature sensor, if any.
  if (this->temperature_sensor_)
  {
    this->temperature_sensor_->add_on_state_callback([this](float state)
    {
      this->current_temperature = state;
      this->publish_state();
    });
    this->current_temperature = this->temperature_sensor_->state;
  } 
  else
  {
    this->current_temperature = NAN;
  }

  // Restore previous state, if any.
  auto restore = this->restore_state_();
  if (restore.has_value())
  {
    restore->apply(this);
  } 
  else
  {
    // Set some arbitrary defaults.
    this->mode = climate::CLIMATE_MODE_OFF;
    this->fan_mode = climate::CLIMATE_FAN_HIGH;
    this->swing_mode = climate::CLIMATE_SWING_OFF;
    this->target_temperature = traits().get_visual_min_temperature();
  }

  this->transmitter_->begin();
}

climate::ClimateTraits GreeClimate::traits()
{
  auto traits = climate::ClimateTraits();

  traits.set_supports_current_temperature(true);
  traits.set_supports_two_point_target_temperature(false);
  if (this->visual_min_temperature_override_.has_value()) 
  {
    traits.set_visual_min_temperature(*this->visual_min_temperature_override_);
  }
  if (this->visual_max_temperature_override_.has_value()) 
  {
    traits.set_visual_max_temperature(*this->visual_max_temperature_override_);
  }
  if (this->visual_target_temperature_step_override_.has_value()) 
  {
    traits.set_visual_temperature_step(*this->visual_target_temperature_step_override_);
  }

  std::set<climate::ClimateMode> climateModes; 
  climateModes.insert(climate::CLIMATE_MODE_OFF);
  climateModes.insert(climate::CLIMATE_MODE_COOL);
  //climateModes.insert(climate::CLIMATE_MODE_HEAT);
  climateModes.insert(climate::CLIMATE_MODE_DRY);
  climateModes.insert(climate::CLIMATE_MODE_FAN_ONLY);
  traits.set_supported_modes(climateModes);

  std::set<climate::ClimateFanMode> climateFanModes; 
  climateFanModes.insert(climate::CLIMATE_FAN_AUTO);
  climateFanModes.insert(climate::CLIMATE_FAN_LOW);
  climateFanModes.insert(climate::CLIMATE_FAN_MEDIUM);
  climateFanModes.insert(climate::CLIMATE_FAN_HIGH);
  //climateFanModes.insert(climate::CLIMATE_FAN_FOCUS);
  traits.set_supported_fan_modes(climateFanModes);

 // std::set<climate::ClimateSwingMode> climateSwingModes;
 // climateSwingModes.insert(climate::CLIMATE_SWING_OFF);
 // climateSwingModes.insert(climate::CLIMATE_SWING_VERTICAL);
 // traits.set_supported_swing_modes(climateSwingModes);

  return traits;
}

void GreeClimate::control(const climate::ClimateCall &call)
{
  if (call.get_mode().has_value())
  {
    setClimateMode(*call.get_mode());
  }
  if (call.get_target_temperature().has_value())
  {
    setTargetTemperature(*call.get_target_temperature());
  }
  if (call.get_fan_mode().has_value())
  {
    setFanMode(*call.get_fan_mode());
  }
//  if (call.get_swing_mode().has_value())
//  {
//    setSwingMode(*call.get_swing_mode());
//  }

  this->transmitter_->send();
}

void GreeClimate::setClimateMode(const climate::ClimateMode climateMode)
{
  switch (climateMode)
  {
 // case climate::CLIMATE_MODE_HEAT:
 //   this->transmitter_->setMode(kGreeHeat);
 //   this->transmitter_->on();
 //   break;
  case climate::CLIMATE_MODE_COOL:
    this->transmitter_->setMode(kGreeCool);
    this->transmitter_->on();
    break;
  case climate::CLIMATE_MODE_DRY:
    this->transmitter_->setMode(kGreeDry);
    this->transmitter_->on();
    break;
  case climate::CLIMATE_MODE_FAN_ONLY:
    this->transmitter_->setMode(kGreeFan);
    this->transmitter_->on();
    break;
  case climate::CLIMATE_MODE_OFF:
    this->transmitter_->off();
    break;
  default:
    ESP_LOGE(TAG, "Invalid climate mode %d", climateMode);
    return;
  }

  this->mode = climateMode;
  this->publish_state();
}

void GreeClimate::setTargetTemperature(const float targetTemperature)
{
  this->transmitter_->setTemp(targetTemperature);

  this->target_temperature = targetTemperature;
  this->publish_state();
}

void GreeClimate::setFanMode(const climate::ClimateFanMode fanMode)
{
    switch (fanMode)
    {
    case climate::CLIMATE_FAN_AUTO:
      this->transmitter_->setFan(kGreeFanAuto);
      this->transmitter_->setTurbo(false);
      break;
    case climate::CLIMATE_FAN_LOW:
      this->transmitter_->setFan(kGreeFanMin);
      this->transmitter_->setTurbo(false);
      break;
    case climate::CLIMATE_FAN_MEDIUM:
      this->transmitter_->setFan(kGreeFanMed);
      this->transmitter_->setTurbo(false);
      break;
    case climate::CLIMATE_FAN_HIGH:
      this->transmitter_->setFan(kGreeFanMax);
      this->transmitter_->setTurbo(false);
      break;
   // case climate::CLIMATE_FAN_FOCUS:
   //   this->transmitter_->setFan(kGreeFanMax);
   //   this->transmitter_->setTurbo(true);
   //   break;
   // default:
      ESP_LOGE(TAG, "Invalid climate fan mode %d", fanMode);
      return;
    }

    this->fan_mode = fanMode;
    this->publish_state();
}

//void GreeClimate::setSwingMode(const climate::ClimateSwingMode swingMode)
//{
//  switch (swingMode)
//  {
//  case climate::CLIMATE_SWING_OFF:
//    this->transmitter_->setSwingVertical(false, kGreeSwingLastPos);
//    break;
//  case climate::CLIMATE_SWING_VERTICAL:
//    this->transmitter_->setSwingVertical(true, kGreeSwingAuto);
//    break;
//  default:
//    ESP_LOGE(TAG, "Invalid climate swing mode %d", swingMode);
//    return;
//  }

//  this->swing_mode = swingMode;
//  this->publish_state();
//}

} // namespace gree
} // namespace esphome
