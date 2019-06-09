#include <linux/input.h>

#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>

using namespace std::chrono;

typedef std::map<int, std::string> MouseDeviceMap;


// Get the list of mouse devices from /proc/bus/input/devices
static
MouseDeviceMap
getMouseDevices()
{
   MouseDeviceMap devices;

   std::regex device_name_re("^N: Name=\"(.*)\"");
   std::regex mouse_event_re("^H: Handlers=mouse\\d.*event(\\d+)\\s*");
   std::regex blank_line_re("^\\s*$");
   std::ifstream dev_f("/proc/bus/input/devices");
   std::string line;
   std::string device_name;
   while (std::getline(dev_f, line))
   {
      std::smatch match_result;
      if (std::regex_match(line, match_result, device_name_re))
      {
         device_name = match_result[1];
      }
      else if (std::regex_match(line, match_result, mouse_event_re))
      {
         if (!device_name.empty())
         {
            devices[stoi(match_result[1])] = device_name;
         }
         device_name.clear();
      }
      else if (std::regex_match(line, match_result, blank_line_re))
      {
         device_name.clear();
      }
   }

   return devices;
}

static
std::ostream &
printMouseDevices(std::ostream &os, const MouseDeviceMap &devices)
{
   for (auto device: devices)
   {
      os << "\t" << device.first << "\t" << device.second << std::endl;
   }
   return os;
}

static
void
usage(const std::string &app_name, const MouseDeviceMap &devices)
{
   printMouseDevices(
           std::cout << "Usage:\n\t" << app_name << " device_number\n\n"
           << "where \"device_number\" is one of:\n\n", devices)
           << std::endl;
}

int
main(int argc, char *argv[])
{
   std::string device_number;
   auto devices = getMouseDevices();

   for (int i = 1; i < argc; ++i)
   {
      if (device_number.empty())
      {
         device_number = argv[i];
      }
      else
      {
         usage(*argv, devices);
         return 1;
      }
   }

   if (device_number.empty())
   {
      usage(*argv, devices);
      return 0;
   }

   // choose mouse device
   MouseDeviceMap::const_iterator device = devices.end();
   try
   {
      device = devices.find(stoi(device_number));
      if (device == devices.end())
      {
         printMouseDevices(
                 std::cout << "No such device \"" << device_number << "\".\n"
                 << "please choose one of:\n\n", devices)
                 << std::endl;
         return 0;
      }
   }
   catch (const std::invalid_argument &e)
   {
      printMouseDevices(
              std::cout << "No such device \"" << device_number << "\".\n"
              << "please choose one of:\n\n", devices)
              << std::endl;
      return 1;
   }

   std::cout << "\nUsing mouse device \"" << device->second << "\"" << std::endl;


   // Read and report `the events from the mouse input device.
   auto mouse_filename = std::string("/dev/input/event") + device_number;
   auto t0 = system_clock::now();
   long x = 0L, y = 0L, dx = 0L, dy = 0L;
   struct input_event event;
   std::ifstream device_file(mouse_filename.c_str(), std::ios::in);
   while (device_file.read(reinterpret_cast<char*>(&event), sizeof(event)))
   {
      if (event.type == EV_REL)
      {
         dx = 0;
         dy = 0;
         if (event.code == 0)
         {
            dx = event.value;
            x += dx;
         }
         else if (event.code == 1)
         {
            dy = event.value;
            y += dy;
         }

         auto event_t = duration<float, std::ratio<1, 1>>(system_clock::time_point{seconds{event.time.tv_sec} + microseconds{event.time.tv_usec}} - t0).count();
         std::cout << event_t
                 << "\tx=" << x
                 << "\ty=" << y
                 << "\tdx=" << dx
                 << "\tdy=" << dy
                 << std::endl;
      }
   }

   return 0;
}

