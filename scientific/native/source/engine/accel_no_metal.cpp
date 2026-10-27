#include "accel_device.hpp"
std::unique_ptr<JipDevice> jip_metal_device(std::string &error){error="Metal is available only on macOS.";return nullptr;}
