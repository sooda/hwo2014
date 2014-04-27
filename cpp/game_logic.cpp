#include "game_logic.h"
#include "protocol.h"
#include <cmath>

using namespace hwo_protocol;

game_logic::game_logic()
  : action_map
    {
      { "join", &game_logic::on_join },
      { "gameStart", &game_logic::on_game_start },
      { "gameInit", &game_logic::on_game_init },
      { "carPositions", &game_logic::on_car_positions },
      { "crash", &game_logic::on_crash },
      { "gameEnd", &game_logic::on_game_end },
      { "error", &game_logic::on_error }
    },
    track { {}, { 0.0 }, 0 },
    mycar(),
    current_tick { -1 }
{
}

game_logic::msg_vector game_logic::react(const jsoncons::json& msg)
{
  const auto& msg_type = msg["msgType"].as<std::string>();
  const auto& data = msg["data"];
  int tick = msg.get("gameTick", -1).as<int>();

  std::cout << "msg tick " << tick << std::endl;
  if (tick != -1)
    current_tick = tick;

  auto action_it = action_map.find(msg_type);
  if (action_it != action_map.end())
  {
    msg_vector act = (action_it->second)(this, data);
    if (tick != -1 && act.size() == 0) {
      std::cout << "BUG: got tick but did no actions" << std::endl;
      act = { make_ping() };
    }
    return act;
  }
  else
  {
    std::cout << "Unknown message type: " << msg_type << std::endl;
    if (tick == -1)
      return {};
    return { make_ping() };
  }
}

game_logic::msg_vector game_logic::on_join(const jsoncons::json& data)
{
  std::cout << "Joined" << std::endl;
  return { };
}

game_logic::msg_vector game_logic::on_game_init(const jsoncons::json& data)
{
  std::cout << "Game init" << std::endl;

  track = data["race"]["track"].as<Track>();
  mycar = Player(&track);
  for (auto& piece: track.track) {
    std::cout << piece << std::endl;
  }
  std::cout << track.track.size() << " pieces in total, lane count " << track.nlanes << std::endl;
  std::cout << "lane dists";
  for (auto& x: track.lanedist)
    std::cout << " " << x;
  std::cout << std::endl;

  auto cars = data["race"]["cars"];
  for (size_t i = 0; i < cars.size(); i++) {
    std::cout << "car "
      << cars[i]["id"]["name"] << " "
      << cars[i]["id"]["color"]
      << std::endl;
  }
  return { };
}

game_logic::msg_vector game_logic::on_game_start(const jsoncons::json& data)
{
  std::cout << "Race started" << std::endl;

  // (a carpositions with no ticks might come before the start so that starts
  // includes the first tick)

  // just go full speed here to estimate the track coefs
  if (current_tick < 1)
    return { make_throttle(1.0, 0) };
  // not started yet? no commands
  return { };
}

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
  double topspeed = speed_for_bend(next);
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
  double dist = now.inPieceDistance;
  int idx = (now.pieceIndex + 1) % track->track.size();
  while (idx != target) {
    dist += track->track[idx].travel(-20.0); // TODO: get current fromcenter or something, now a conservative guess
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
  return 4.2;
}

int Player::next_bend(int curridx) const {
  while (track->track[curridx].angle == 0)
    curridx = (curridx + 1) % track->track.size();
  return curridx;
}

game_logic::msg_vector game_logic::on_car_positions(const jsoncons::json& data)
{
  std::cout << "Position tick";

  auto positions = data.as<std::vector<CarPosition>>();
  for (CarPosition& p: positions) {
    std::cout << " " << p.name << ":" << p.color
      << "=(" << p.pieceIndex << "," << p.inPieceDistance << ")";
  }
  std::cout << std::endl;

  CarPosition& now = positions[0]; // XXX parse my color

  double lanedist = track.lanedist[now.startLane];
  double angspeed = mycar.prev.angle - now.angle;

  mycar.update(now);

  double throttle = compute_throttle(now);

  std::cout
    << "ticks " << mycar.nticks
    << ", current index " << now.pieceIndex
    << ", current travel " << track.track[now.pieceIndex].travel(lanedist)
    << ", piece travel " << now.inPieceDistance
    << ", total traveled: " << mycar.tottravel
    << ", this speed: " << mycar.curspeed
    << ", angle: " << now.angle
    << ", angular speed: " << angspeed
    << ", throttle: " << throttle
    << std::endl;

  std::cerr << mycar.nticks
    << " " << mycar.tottravel
    << " " << mycar.curspeed
    << " " << now.pieceIndex
    << " " << now.angle
    << " " << angspeed
    << " " << throttle
    << std::endl;

  mycar.prev = now;
  mycar.nticks++;

  // first positions may come before start
  if (current_tick == -1)
    return {};

  return { make_throttle(throttle, mycar.nticks - 1) };
}

double game_logic::compute_throttle(const CarPosition& now) const
{
  if (mycar.nticks < 2)
    return 1.0; // for coef estimation
  else
    return mycar.compute_throttle(now);
#if 0
  int estim_interval = 10 * 60;
  double speed = std::min((mycar.nticks / estim_interval + 1) / 10.0, 0.7);
  return mycar.nticks%estim_interval < estim_interval/2 ? speed : 0.0;
#endif
}

game_logic::msg_vector game_logic::on_crash(const jsoncons::json& data)
{
  std::cout << "Someone crashed" << std::endl;
  return { make_ping() };
}

game_logic::msg_vector game_logic::on_game_end(const jsoncons::json& data)
{
  std::cout << "Race ended" << std::endl;
  return { make_ping() };
}

game_logic::msg_vector game_logic::on_error(const jsoncons::json& data)
{
  std::cout << "Error: " << data.to_string() << std::endl;
  return { make_ping() };
}
