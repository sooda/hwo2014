#include "player.h"

void Player::update(const CarPosition& now) {
  estimate_coefs(now);
  curspeed = compute_travel(now);
  tottravel += curspeed;
}

void Player::estimate_coefs(const CarPosition& now) {
  if (nticks == 2) {
    double initial_thrust = 1.0;
    double x1 = prev.inPieceDistance;
    double x2 = now.inPieceDistance;
    // v1 = x1, as x0 = 0
    double v2 = x2 - x1;

    power = x1 / initial_thrust; // v += power * thrust, initially v zero so no drag
    drag = (v2 - x1) / x1; // v2 = drag * v1 + accel, accel = increase in one step = x1
  }
}

double Player::compute_travel(const CarPosition& now) const {
  // TODO lane switching
  double lanedist = track->lanedist[now.startLane];

  double travel;
  if (now.pieceIndex == prev.pieceIndex) {
    travel = now.inPieceDistance - prev.inPieceDistance;
  } else {
    // changed piece between ticks
    double last_remaining = track->track[prev.pieceIndex].travel(lanedist) - prev.inPieceDistance;
    double in_this = now.inPieceDistance;
    travel = last_remaining + in_this;
  }
  return travel;
}

double Player::compute_throttle(const CarPosition& now) const {
#if 0
  return throttle_for_speed((nticks / 200) * topspeed() / 10.0);
#endif
  // assume so far that no bend after the next one needs remarkably slower speed
  int next = next_bend(now.pieceIndex);
  double topspeed = speed_for_bend(track->track[next].radius);
  double dist_to_next = dist_to_piece(now, next);

  if (dist_to_next == 0.0) // already in this bend
    return throttle_for_speed(topspeed);

  int ticks = ticks_to_slow_down(curspeed, topspeed);
  double brake_dist = brake_travel(curspeed, ticks);

  if (dist_to_next > brake_dist)
    return 1.0;
  return 0.0;
}

double Player::dist_to_piece(const CarPosition& now, int target) const {
  if (now.pieceIndex == target)
    return 0.0;

  double lanedist = track->lanedist[now.startLane]; // close enough estimate to hold current lane
  double dist = track->track[now.pieceIndex].travel(lanedist) - now.inPieceDistance;

  int idx = (now.pieceIndex + 1) % track->track.size();
  while (idx != target) {
    dist += track->track[idx].travel(lanedist);
    idx = (idx + 1) % track->track.size();
  }

  return dist;
}

double Player::throttle_for_speed(double speed) const {
  double thr = (speed - drag * curspeed) / power;
  return std::min(std::max(thr, 0.0), 1.0);
}

double Player::topspeed() const {
  // v_n+1 = v_n - (1-d)v_n + p*t
  //     0 =      -(1-d)v_n + p*t
  return power * 1.0 / (1 - drag);
}

double Player::brake_travel(double startspeed, int ticks) const {
  // sum of the first n terms of a geometric series
  return (1 - std::pow(drag, ticks)) / (1 - drag) * startspeed;
}

int Player::ticks_to_slow_down(double cur, double target) const {
  // reach slower speed from current:
  // d^n * cur = target
  // log d^n = log target / cur
  return ceil(log(target / cur) / log(drag));
}

double Player::speed_for_bend(double radius) const {
  return 0.6 * std::sqrt(radius);
}

int Player::next_bend(int curridx) const {
  while (track->track[curridx].angle == 0)
    curridx = (curridx + 1) % track->track.size();
  return curridx;
}
