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

#include "SDL2/SDL.h"
#include <RobotRaconteur.h>
#include <RobotRaconteurCompanion/StdRobDef/StdRobDefAll.h>

#include <boost/uuid/uuid_io.hpp>

#pragma once

namespace robotraconteur_joystick_driver {

namespace RR = RobotRaconteur;
namespace rrjoy = com::robotraconteur::hid::joystick;
namespace rrsensordata = com::robotraconteur::sensordata;

rrjoy::JoystickInfoPtr fill_joystick_info(SDL_Joystick *joy, uint32_t);

rrjoy::JoystickStatePtr fill_joystick_state(SDL_Joystick *joy);

rrjoy::GamepadStatePtr fill_gamepad_state(SDL_Joystick *joy);

class JoystickImpl : public rrjoy::Joystick_default_impl,
                     public RR::IRRServiceObject {
protected:
  uint32_t id = 0;
  SDL_Joystick *joy = nullptr;
  SDL_GameController *pad = nullptr;

  SDL_Haptic *haptic = nullptr;

  bool has_rumble = false;
  bool has_ff = false;

  SDL_HapticConstant constant_effect;
  int constant_effect_id = 0;

  RR_SHARED_PTR<RR::BroadcastDownsampler> downsampler;

  uint64_t seqno = 0;

  rrjoy::JoystickInfoPtr joy_info;

public:
  JoystickImpl();

  void Open(uint32_t id, rrjoy::JoystickInfoPtr joy_info);

  virtual void RRServiceObjectInit(RR_WEAK_PTR<RR::ServerContext> context,
                                   const std::string &service_path);

  virtual rrjoy::JoystickInfoPtr get_joystick_info();

  void SendState();

  virtual void rumble(double intensity, double duration);

  void force_feedback(const com::robotraconteur::geometry::Vector2 &force,
                      double duration);

  virtual ~JoystickImpl();

  virtual uint32_t get_update_downsample();
  virtual void set_update_downsample(uint32_t value);

  virtual double get_update_rate();
};

} // namespace robotraconteur_joystick_driver
