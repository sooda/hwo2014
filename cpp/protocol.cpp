#include "protocol.h"

#include <sstream>
namespace hwo_protocol
{

  jsoncons::json make_request(const std::string& msg_type, const jsoncons::json& data)
  {
    jsoncons::json r;
    r["msgType"] = msg_type;
    r["data"] = data;
    return r;
  }

  jsoncons::json make_join(const std::string& name, const std::string& key)
  {
    jsoncons::json data;
    data["name"] = name;
    data["key"] = key;
    return make_request("join", data);
  }

  jsoncons::json make_create_single(const std::string& name, const std::string& key, const std::string& track)
  {
    jsoncons::json data;
    data["botId"] = jsoncons::json();
    data["botId"]["name"] = name;
    data["botId"]["key"] = key;
    data["trackName"] = track;
    data["carCount"] = 1;
    return make_request("createRace", data);
  }

  jsoncons::json make_join_race(const std::string& name, const std::string& key, const std::string& track, const std::string& pwd, const std::string& carcount)
  {
    jsoncons::json data;
    data["botId"] = jsoncons::json();
    data["botId"]["name"] = name;
    data["botId"]["key"] = key;
    data["trackName"] = track;
    data["password"] = pwd;
    data["carCount"] = std::stoi(carcount);
    return make_request("joinRace", data);
  }

  jsoncons::json make_ping()
  {
    return make_request("ping", jsoncons::null_type());
  }

  jsoncons::json make_throttle(double throttle, int tick)
  {
    jsoncons::json req(make_request("throttle", throttle));
    req["gameTick"] = tick;
    return req;
  }

  jsoncons::json make_lane_change(const std::string& dir)
  {
    return make_request("switchLane", dir);
  }
}  // namespace hwo_protocol
