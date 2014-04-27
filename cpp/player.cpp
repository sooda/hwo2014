#include "player.h"

void Player::update(const CarPosition& now) {
  if (nticks == 0) {
    // HACK. CI starts in random positions
    // don't mess up speed etc starting in the middle of something
    // just get prev ok during this tick
    return;
  }

  curspeed = compute_travel(now);
  estimate_coefs(now);
  prevspeed = curspeed;
  tottravel += curspeed;
}

void Player::estimate_coefs(const CarPosition& now) {
  if (nticks == COEF_MEAS_TICKS) {
    double initial_thrust = 1.0;
    // start pos is not always 0; speed in tick delta units
    double x1 = prevspeed;//prev.inPieceDistance;
    double x2 = prevspeed+curspeed;//now.inPieceDistance;
    // v1 = x1, as x0 = 0
    double v2 = x2 - x1;

    power = x1 / initial_thrust; // v += power * thrust, initially v zero so no drag
    drag = (v2 - x1) / x1; // v2 = drag * v1 + accel, accel = increase in one step = x1
    std::cout << "COEF: p=" << power << " d=" << drag << std::endl;
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
  // safest throttle based on all the future track.
  // probably do not need to look ahead the whole track, but what the hell
  double best_throttle = 1.0;
  // track has >= 1 pieces for sure
  for (size_t i = 0; i < track->track.size() - 1; i++) {
    double thr = throttle_for_piece(now, i);
    best_throttle = std::min(best_throttle, thr);
  }
  return best_throttle;
}

double Player::throttle_for_piece(const CarPosition& now, int lookahead) const {
  int next = (now.pieceIndex + lookahead) % track->track.size();
  // straights do not need braking
  if (track->track[next].length != 0.0)
    return 1.0;

  double topspeed = speed_for_bend(track->track[next].radius);
  if (lookahead == 0) // already in this bend
    return throttle_for_speed(topspeed);

  double dist_to_next = dist_to_piece(now, next);
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
  return 0.62 * std::sqrt(radius);
}

int Player::next_bend(int curridx) const {
  while (track->track[curridx].angle == 0)
    curridx = (curridx + 1) % track->track.size();
  return curridx;
}

void Player::endtick(const CarPosition& now) {
  prev = now;
  prevspeed = curspeed;
  nticks++;
}

