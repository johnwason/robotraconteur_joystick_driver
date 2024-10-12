// Copyright 2020 Wason Technology, LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "joystick_impl.h"

#include <RobotRaconteurCompanion/Util/SensorDataUtil.h>

namespace robotraconteur_joystick_driver {

void fill_joystick_info(SDL_Joystick *joy, uint32_t id,
                        rrjoy::JoystickInfoPtr joy_info) {
  joy_info->id = id;
  joy_info->axes_count = (uint32_t)SDL_JoystickNumAxes(joy);
  joy_info->button_count = (uint32_t)SDL_JoystickNumButtons(joy);
  joy_info->hat_count = (uint32_t)SDL_JoystickNumHats(joy);

  joy_info->joystick_capabilities = 0;
  if (SDL_IsGameController(id)) {
    joy_info->joystick_capabilities |=
        rrjoy::JoystickCapabilities::standard_gamepad;
  }

  if (SDL_JoystickIsHaptic(joy)) {
    SDL_Haptic *haptic = SDL_HapticOpenFromJoystick(joy);
    std::string sdl_error(SDL_GetError());
    if (haptic) {
      unsigned int haptic_query = SDL_HapticQuery(haptic);
      if (haptic_query) {
        if (SDL_IsGameController(id)) {
          if ((haptic_query & (SDL_HAPTIC_SINE | SDL_HAPTIC_LEFTRIGHT)) != 0) {
            joy_info->joystick_capabilities |=
                rrjoy::JoystickCapabilities::rumble;
          } else {
            if ((haptic_query & (SDL_HAPTIC_CARTESIAN)) != 0) {
              joy_info->joystick_capabilities |=
                  rrjoy::JoystickCapabilities::force_feedback;
            }
          }
        }
      }

      SDL_HapticClose(haptic);
    }
  }

  joy_info->joystick_device_vendor = SDL_JoystickGetVendor(joy);
  joy_info->joystick_device_product = SDL_JoystickGetProduct(joy);
  joy_info->joystick_device_version = SDL_JoystickGetProductVersion(joy);
  SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
  memcpy(joy_info->joystick_uuid.a.data(), guid.data, sizeof(guid.data));
}

rrjoy::JoystickStatePtr fill_joystick_state(SDL_Joystick *joy) {
  rrjoy::JoystickStatePtr joy_state(new rrjoy::JoystickState());
  int axes_count = SDL_JoystickNumAxes(joy);
  joy_state->axes = RR::AllocateRRArray<int16_t>((size_t)axes_count);
  for (int i = 0; i < axes_count; i++) {
    joy_state->axes->at(i) = SDL_JoystickGetAxis(joy, i);
  }

  int button_count = SDL_JoystickNumButtons(joy);
  joy_state->buttons = RR::AllocateRRArray<uint8_t>((size_t)button_count);
  for (int i = 0; i < button_count; i++) {
    joy_state->buttons->at(i) = SDL_JoystickGetButton(joy, i);
  }

  int hat_count = SDL_JoystickNumHats(joy);
  joy_state->hats = RR::AllocateRRArray<uint8_t>((size_t)hat_count);
  for (int i = 0; i < hat_count; i++) {
    joy_state->hats->at(i) = SDL_JoystickGetHat(joy, i);
  }

  return joy_state;
}

rrjoy::GamepadStatePtr fill_gamepad_state(SDL_GameController *joy) {
  rrjoy::GamepadStatePtr joy_state(new rrjoy::GamepadState());
  joy_state->left_x = SDL_GameControllerGetAxis(joy, SDL_CONTROLLER_AXIS_LEFTX);
  joy_state->left_y = SDL_GameControllerGetAxis(joy, SDL_CONTROLLER_AXIS_LEFTY);
  joy_state->right_x =
      SDL_GameControllerGetAxis(joy, SDL_CONTROLLER_AXIS_RIGHTX);
  joy_state->right_y =
      SDL_GameControllerGetAxis(joy, SDL_CONTROLLER_AXIS_RIGHTY);
  joy_state->trigger_left =
      SDL_GameControllerGetAxis(joy, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
  joy_state->trigger_right =
      SDL_GameControllerGetAxis(joy, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

  joy_state->buttons = 0;
  for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
    if (SDL_GameControllerGetButton(joy, (SDL_GameControllerButton)i)) {
      uint16_t button_flag = (1 << i);
      joy_state->buttons |= button_flag;
    }
  }

  return joy_state;
}

JoystickImpl::JoystickImpl() {}

void JoystickImpl::Open(
    uint32_t id, com::robotraconteur::hid::joystick::JoystickInfoPtr joy_info) {

  boost::mutex::scoped_lock lock(this_lock);

  SDL_Joystick *joy = nullptr;
  SDL_GameController *pad = nullptr;
  if (SDL_IsGameController(id)) {
    pad = SDL_GameControllerOpen(id);
    if (pad == nullptr) {
      std::string sdl_error(SDL_GetError());
      std::string msg = "Could not open joystick ";
      msg += id + ": " + sdl_error;
      throw RR::SystemResourceException(msg);
    }
    joy = SDL_JoystickOpen((int)id);
    if (joy == nullptr) {
      SDL_GameControllerClose(pad);
      std::string sdl_error(SDL_GetError());
      std::string msg = "Could not open joystick ";
      msg += id + ": " + sdl_error;
      throw RR::SystemResourceException(msg);
    }
  } else {
    joy = SDL_JoystickOpen((int)id);
    if (joy == nullptr) {
      std::string sdl_error(SDL_GetError());
      std::string msg = "Could not open joystick ";
      msg += id + ": " + sdl_error;
      throw RR::SystemResourceException(msg);
    }
  }

  SDL_Haptic *haptic = nullptr;
  if (SDL_JoystickIsHaptic(joy)) {
    haptic = SDL_HapticOpenFromJoystick(joy);
    if (haptic) {
      if (SDL_HapticRumbleSupported(haptic)) {
        if (SDL_HapticRumbleInit(haptic) == 0) {
          this->has_rumble = true;
        }
      }

      memset(&this->constant_effect, 0, sizeof(this->constant_effect));
      this->constant_effect.type = SDL_HAPTIC_CONSTANT;
      if (SDL_HapticEffectSupported(
              haptic, (SDL_HapticEffect *)&this->constant_effect)) {
        this->constant_effect.type = SDL_HAPTIC_CONSTANT;
        this->constant_effect.length = 1000;
        this->constant_effect.level = 100;
        this->constant_effect_id = SDL_HapticNewEffect(
            haptic, (SDL_HapticEffect *)&this->constant_effect);
        this->has_ff = true;
      }
    }
  }

  this->id = id;
  this->joy = joy;
  this->pad = pad;
  this->haptic = haptic;

  fill_joystick_info(joy, id, joy_info);
  this->joy_info = joy_info;
}

void JoystickImpl::RRServiceObjectInit(RR_WEAK_PTR<RR::ServerContext> context,
                                       const std::string &service_path) {
  downsampler = boost::make_shared<RR::BroadcastDownsampler>();
  downsampler->Init(context.lock());
  downsampler->AddWireBroadcaster(rrvar_joystick_state);
  downsampler->AddWireBroadcaster(rrvar_gamepad_state);
  downsampler->AddPipeBroadcaster(rrvar_joystick_sensor_data);
  rrvar_joystick_sensor_data->SetMaxBacklog(10);
}

rrjoy::JoystickInfoPtr JoystickImpl::get_joystick_info() {
  boost::mutex::scoped_lock lock(this_lock);
  return joy_info;
}

void JoystickImpl::SendState() {
  boost::mutex::scoped_lock lock(this_lock);

  seqno++;

  SDL_JoystickUpdate();

  if (!downsampler) {
    return;
  }

  RR::BroadcastDownsamplerStep step(downsampler);

  if (!rrvar_joystick_state) {
    return;
  }

  auto joy_state = fill_joystick_state(joy);
  rrvar_joystick_state->SetOutValue(joy_state);

  if (!rrvar_gamepad_state) {
    return;
  }

  auto pad_state = fill_gamepad_state(pad);
  rrvar_gamepad_state->SetOutValue(pad_state);

  if (!rrvar_joystick_sensor_data) {
    return;
  }

  rrjoy::JoystickStateSensorDataPtr joy_sensor_data(
      new rrjoy::JoystickStateSensorData());
  joy_sensor_data->data_header =
      RobotRaconteur::Companion::Util::FillSensorDataHeader(
          RR::RobotRaconteurNode::sp(), joy_info->device_info, seqno);
  joy_sensor_data->joystick_state = joy_state;
  joy_sensor_data->gamepad_state = pad_state;

  rrvar_joystick_sensor_data->AsyncSendPacket(joy_sensor_data, []() {});
}

void JoystickImpl::rumble(double intensity, double duration) {
  boost::mutex::scoped_lock lock(this_lock);
  if (has_rumble) {
    SDL_HapticRumblePlay(this->haptic, (float)intensity,
                         boost::numeric_cast<uint32_t>(duration * 1000.0));
  }
}

void JoystickImpl::force_feedback(
    const com::robotraconteur::geometry::Vector2 &force, double duration) {
  boost::mutex::scoped_lock lock(this_lock);
  if (has_ff) {
    this->constant_effect.type = SDL_HAPTIC_CONSTANT;
    this->constant_effect.direction.type = SDL_HAPTIC_CARTESIAN;
    this->constant_effect.direction.dir[0] =
        boost::numeric_cast<int32_t>(force.s.x * 10000.0);
    this->constant_effect.direction.dir[1] =
        boost::numeric_cast<int32_t>(force.s.y * 10000.0);
    this->constant_effect.length =
        boost::numeric_cast<uint32_t>(duration * 1000.0);
    this->constant_effect.level = boost::numeric_cast<int16_t>(
        sqrt(pow(force.s.x, 2.0) + pow(force.s.y, 2.0)) *
        boost::numeric_cast<double>(std::numeric_limits<int16_t>::max()));
    if (SDL_HapticUpdateEffect(haptic, this->constant_effect_id,
                               (SDL_HapticEffect *)&this->constant_effect) !=
        0) {
      throw RR::SystemResourceException("Could not set force feedback");
    }
  }
}

uint32_t JoystickImpl::get_update_downsample() {
  uint32_t local_ep =
      RR::ServerEndpoint::GetCurrentEndpoint()->GetLocalEndpoint();
  if (downsampler) {
    return downsampler->GetClientDownsample(local_ep);
  } else {
    return 0;
  }
}

void JoystickImpl::set_update_downsample(uint32_t value) {
  uint32_t local_ep =
      RR::ServerEndpoint::GetCurrentEndpoint()->GetLocalEndpoint();
  if (downsampler) {
    downsampler->SetClientDownsample(local_ep, value);
  }
}

double JoystickImpl::get_update_rate() { return 100.0; }

JoystickImpl::~JoystickImpl() {

  if (has_ff) {
    SDL_HapticDestroyEffect(haptic, constant_effect_id);
  }

  if (haptic) {
    SDL_HapticClose(haptic);
  }

  if (pad) {
    SDL_GameControllerClose(pad);
  } else {
    if (joy) {
      SDL_JoystickClose(joy);
    }
  }
}

} // namespace robotraconteur_joystick_driver
