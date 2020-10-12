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

namespace robotraconteur_joystick_driver
{

void print_joystick_info(bool yaml = false, std::ostream& os = std::cout)
{
    int num_joysticks = SDL_NumJoysticks();

    size_t name_len = 0;
    for (int i=0; i<num_joysticks; i++)
    {
        const char* name_cstr = SDL_JoystickNameForIndex(i);        
        if (name_cstr)
        {
            size_t name_cstr_len = strlen(name_cstr);
            if (name_cstr_len > name_len)
            {
                name_len = name_cstr_len;
            }
        }
    }

    if (!yaml)
    {
    os << "Detected " << num_joysticks << " Joysticks" << std::endl << std::endl;

    os   << std::setfill(' ') << std::left
                << "ID " // 3
                << std::setw(name_len + 1) << "Name" // 1 + name_len
                << std::setw(37) << "DeviceLocalGUID"  // 37
                << std::setw(0) <<  "IsGamepad " // 10
                << "NumAxes " // 8
                << "NumButtons " // 11
                << "NumHats " // 8
                << "HasRumble " // 10
                << "HasFF" // 6
                << std::endl;
    }
    for (int i=0; i<num_joysticks; i++)
    {
        SDL_Joystick* joy = SDL_JoystickOpen(i);
        if (!joy)
        {
            std::cerr << i << " Error: could not open joystick" << std::endl;
            continue;
        }

        SDL_JoystickGUID joy_guid = SDL_JoystickGetGUID(joy);
        boost::uuids::uuid joy_uuid;
        memcpy(&joy_uuid, joy_guid.data, sizeof(joy_uuid));
        std::string joy_guid_str = boost::lexical_cast<std::string>(joy_uuid);

        bool has_rumble = false;
        bool has_ff = false;

        if (SDL_JoystickIsHaptic(joy))
        {
            SDL_Haptic* haptic = SDL_HapticOpenFromJoystick(joy);
            std::string sdl_error(SDL_GetError());
            if (haptic)
            {

                if (SDL_HapticRumbleSupported(haptic))
                {
                    has_rumble = true;
                }

                SDL_HapticConstant constant_effect;
                constant_effect.type = SDL_HAPTIC_CONSTANT;
                if (SDL_HapticEffectSupported(haptic, (SDL_HapticEffect*)&constant_effect))
                {
                    has_ff = true;
                }                

                SDL_HapticClose(haptic);
            }
        }

        if (!yaml)
        {
        os << std::setfill(' ') << std::left
                  << std::setw(3) << i
                  << std::setw(name_len+1) << SDL_JoystickName(joy)                  
                  << std::setw(37) << joy_guid_str
                  << std::setw(10) << (SDL_IsGameController(i) ? "yes" : "no")
                  << std::setw(8) << SDL_JoystickNumAxes(joy)
                  << std::setw(11) << SDL_JoystickNumButtons(joy)
                  << std::setw(8) << SDL_JoystickNumHats(joy)
                  << std::setw(10) << (has_rumble ? "yes" : "no")
                  << std::setw(6) << (has_ff ? "yes" : "no")
                  << std::endl;
        }
        else
        {
            os
            << "- id: " << i << std::endl
            << "  name: " << SDL_JoystickName(joy) << std::endl
            << "  guid: " << joy_guid_str << std::endl
            << "  is_gamepad: " << (SDL_IsGameController(i) ? "true" : "false") << std::endl
            << "  num_axes: " << SDL_JoystickNumAxes(joy) << std::endl
            << "  num_buttons: " << SDL_JoystickNumButtons(joy) << std::endl
            << "  num_hats: " << SDL_JoystickNumHats(joy) << std::endl
            << "  has_rumble: " << (has_rumble ? "true" : "false") << std::endl
            << "  has_ff: " << (has_ff ? "true" : "false") << std::endl;
        }

        SDL_JoystickClose(joy);
    }
}

void identify_joystick()
{
    int num_joysticks = SDL_NumJoysticks();

    std::vector<SDL_Joystick*> joys(num_joysticks);
    for (int i=0; i<num_joysticks; i++)
    {
        joys[i] = SDL_JoystickOpen(i);
    }

    std::cout << "Press and hold button on joystick" << std::endl;

    bool found = false;
    for (int j=0;  j<25 && !found; j++)
    {
        SDL_JoystickUpdate();
        for (int i=0; i<num_joysticks && !found; i++)
        {
            if (joys[i] == NULL)
            {
                continue;
            }
            int num_buttons = SDL_JoystickNumButtons(joys[i]);
            for (int k=0; k<num_buttons && !found; k++)
            {
                if (SDL_JoystickGetButton(joys[i],k) != 0)
                {
                    std::cout << "Detected joystick: " << i << std::endl;
                    found=true;
                    break;                    
                }
            }
        }
        
        boost::this_thread::sleep(boost::posix_time::milliseconds(200));
    }

    if (!found)
    {
        std::cout << "No joystick detected" << std::endl;
    }

    for (int i=0; i<num_joysticks; i++)
    {
        SDL_Joystick* joy = joys[i];
        if (joy)
        {
            SDL_JoystickClose(joy);
        }
    }
}

}

bool keepgoing = true;

int main(int argc, char* argv[])
{

    namespace RR = RobotRaconteur;
    using namespace robotraconteur_joystick_driver;
    using namespace RobotRaconteur;
    namespace po = boost::program_options;

    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_NOPARACHUTE) < 0)
    {
        std::cerr << "Could not initialize SDL2, aborting" << std::endl;
        return 1;
    }

    RR_BOOST_ASIO_IO_CONTEXT signal_context;
    boost::asio::signal_set signals(signal_context, SIGINT, SIGTERM);
    signals.async_wait([](const boost::system::error_code& error,int signal_number) {
        keepgoing=false;
    });

    try
    {

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce this message")
            ("list", "list available joysticks")
            ("list-yaml", "list available joysticks in yaml format")
            ("list-yaml-save", po::value<std::string>(), "save list of available joysticks in yaml format")
            ("identify", "identify joystick by holding a button")
            ("joystick-id", po::value<uint32_t>(), "joystick ID");

        //
        //robotraconteur_joystick_driver::identify_joystick();

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << "Robot Raconteur Joystick Driver Version 0.1.0" << std::endl << std::endl;
            std::cout << desc << std::endl;
            return 1;
        }

        if (vm.count("list"))
        {
            robotraconteur_joystick_driver::print_joystick_info();
            return 0;
        }

        if (vm.count("list-yaml"))
        {
            robotraconteur_joystick_driver::print_joystick_info(true);
            return 0;
        }

        if (vm.count("list-yaml-save"))
        {
            std::string fname = vm["list-yaml-save"].as<std::string>();
            std::ofstream o(fname.c_str());
            robotraconteur_joystick_driver::print_joystick_info(true, o);
            return 0;
        }

        if (vm.count("identify"))
        {
            robotraconteur_joystick_driver::identify_joystick();
            return 0;
        }

        uint32_t joy_id = 0;

        if(vm.count("joystick-id"))
        {
            joy_id = vm["joystick-id"].as<uint32_t>();
        }

        auto joy_impl = boost::make_shared<JoystickImpl>();
        joy_impl->Open(joy_id);

        std::string node_name = "com.robotraconteur.hid.joystick";
        node_name += boost::lexical_cast<std::string>(joy_id);

        RobotRaconteur::Companion::RegisterStdRobDefServiceTypes();
        RR::ServerNodeSetup node_setup(std::vector<RR::ServiceFactoryPtr>(), node_name, 64234);

        RR::RobotRaconteurNode::s()->RegisterService("joystick", "com.robotraconteur.hid.joystick", joy_impl);

        RR::RatePtr rate = RobotRaconteurNode::s()->CreateRate(100.0);

        std::cerr << "Robot Raconteur Joystick Driver Running Joystick ID: " << joy_id<< ", press Ctrl-C to quit" << std::endl;

        while (keepgoing)
        {
            joy_impl->SendState();
            rate->Sleep();
            signal_context.poll();
        }

    }
    catch (std::exception& e)
    {
        std::cerr << "error: robotraconteur_joystick_driver: " << e.what() << std::endl;
        SDL_Quit();
        return 1;
    }


    SDL_Quit();
    return 0;

}