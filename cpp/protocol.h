#ifndef HWO_PROTOCOL_H
#define HWO_PROTOCOL_H

#include <string>
#include <iostream>
#include <jsoncons/json.hpp>

namespace hwo_protocol
{
  jsoncons::json make_request(const std::string& msg_type, const jsoncons::json& data);
  jsoncons::json make_join(const std::string& name, const std::string& key);
  jsoncons::json make_create_single(const std::string& name, const std::string& key, const std::string& track);
  jsoncons::json make_ping();
  jsoncons::json make_throttle(double throttle, int tick);
  jsoncons::json make_lane_change(const std::string& dir);
}

#endif
