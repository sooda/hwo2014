#include "protocol.h"

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

  jsoncons::json make_ping()
  {
    return make_request("ping", jsoncons::null_type());
  }

  jsoncons::json make_throttle(double throttle)
  {
    return make_request("throttle", throttle);
  }

}  // namespace hwo_protocol
