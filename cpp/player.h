#ifndef PLAYER_H
#define PLAYER_H

#include "game_objs.h"

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

#endif