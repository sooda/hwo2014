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

  const Track* track;


  Player(const Track* track) : prev{ "", "", 0.0, 0, 0.0, 0, 0 },
    tottravel(0.0), nticks(0),
    power(0.0), drag(0.0), curspeed(0.0), track(track)
    {}
  Player() : Player(nullptr) {}
  double compute_throttle(const CarPosition& now) const;
  void update(const CarPosition& now);
  void estimate_coefs(const CarPosition& now);
  double throttle_for_speed(double speed) const;
  double compute_travel(const CarPosition& now) const;
  double topspeed() const;
  double speed_for_bend(double radius) const;
  double brake_travel(double startspeed, int ticks) const;
  int ticks_to_slow_down(double cur, double target) const;
  int next_bend(int curridx) const;
  double dist_to_piece(const CarPosition& now, int target) const;
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

  double compute_throttle(const CarPosition& now) const;

  Track track;
  Player mycar;
  int current_tick;
};

#endif
