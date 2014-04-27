#ifndef HWO_GAME_LOGIC_H
#define HWO_GAME_LOGIC_H

#include "game_objs.h"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <iostream>
#include <jsoncons/json.hpp>

struct Player {
  CarPosition prev;
  double tottravel;
  int nticks;

  double power, drag;
  double curspeed;


  Player() : prev{ "", "", 0.0, 0, 0.0, 0, 0 },
    tottravel(0.0), nticks(0),
    power(0.0), drag(0.0), curspeed(0.0)
    {}
  double compute_throttle() const;
  void update(const Track& track, const CarPosition& now);
  void estimate_coefs(const CarPosition& now);
  double throttle_for_speed(double speed) const;
  double compute_travel(const Track& track, const CarPosition& now) const;
  double topspeed() const;
};

class game_logic
{
public:
  typedef std::vector<jsoncons::json> msg_vector;

  game_logic();
  msg_vector react(const jsoncons::json& msg);

private:
  typedef std::function<msg_vector(game_logic*, const jsoncons::json&)> action_fun;
  const std::map<std::string, action_fun> action_map;

  msg_vector on_join(const jsoncons::json& data);
  msg_vector on_game_start(const jsoncons::json& data);
  msg_vector on_game_init(const jsoncons::json& data);
  msg_vector on_car_positions(const jsoncons::json& data);
  msg_vector on_crash(const jsoncons::json& data);
  msg_vector on_game_end(const jsoncons::json& data);
  msg_vector on_error(const jsoncons::json& data);

  double compute_throttle() const;

  Track track;
  Player mycar;
  int current_tick;
};

#endif
