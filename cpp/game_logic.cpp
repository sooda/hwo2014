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
      { "error", &game_logic::on_error },
      { "yourCar", &game_logic::on_your_car }
    },
    track { {}, { 0.0 }, 0 },
    mycar(),
    current_tick { -1 },
    mycolor(""),
    lane_gonna_change(false),
    lane_changing(true)
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

game_logic::msg_vector game_logic::on_car_positions(const jsoncons::json& data)
{
  std::cout << "Position tick";

  auto positions = data.as<std::vector<CarPosition>>();
  CarPosition now;
  for (CarPosition& p: positions) {
    std::cout << " " << p.name << ":" << p.color
      << "=(" << p.pieceIndex << "," << p.inPieceDistance << ")";
    if (p.color == mycolor)
      now = p;
  }
  std::cout << std::endl;

  double lanedist = track.lanedist[now.startLane];
  double angspeed = mycar.prev.angle - now.angle;

  if (now.startLane != now.endLane)
    lane_changing = true;

  if (now.startLane == now.endLane && lane_changing) {
    lane_changing = false;
    lane_gonna_change = false;
  }

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

  jsoncons::json msg = make_throttle(throttle, mycar.nticks);
  if (mycar.nticks >= Player::COEF_MEAS_TICKS && !lane_gonna_change) {
    int lane_change = need_lane_change(now);
    if (lane_change) {
      msg = make_lane_change(lane_change == -1 ? "Left" : "Right");
      lane_gonna_change = true;
    }
  }

  mycar.endtick(now);

  // first positions may come before start
  if (current_tick == -1)
    return {};
  return { msg };
}

int game_logic::need_lane_change(const CarPosition& now) const {
  // predict if should change the lane to left or right before a turn.
  // does not take into account any lengths or such, so may decide wrong
  // e.g. if there is a longer bend just after the next one

  size_t next_switch = 0, then_switch = 0;
  double left_travel = 0.0, right_travel = 0.0;
  int nbends = 0;

  for (size_t i = 1; i < track.track.size() - 2; i++) {
    size_t n = (now.pieceIndex + i) % track.track.size();
    const Piece& p = track.track[n];

    if (p.switch_ && next_switch == 0) {
      next_switch = i;
    } else if (p.switch_ && next_switch != 0) {
      for (size_t j = next_switch; j < i; j++) { // or: j <= i?
        size_t m = (now.pieceIndex + j) % track.track.size();
        const Piece& q = track.track[m];
        left_travel  += q.travel(track.lanedist[std::max(now.endLane-1, 0)]);
        right_travel += q.travel(track.lanedist[std::min(now.endLane+1, track.nlanes-1)]);
        if (q.length == 0.0)
          nbends++;
      }
      then_switch = i;
      break;
    }
  }
  std::cout << "UGUU " << next_switch << " " << then_switch << " " << left_travel << " " << right_travel << std::endl;

  bool target_right = right_travel < left_travel;

  if (next_switch != 0 && nbends != 0) {
    // can switch because there is a switch piece
    //
    // positive lane dist is to the right, thus max marks the rightmost, min leftmost
    int min_lane = 0;
    int max_lane = track.nlanes-1;

    // we should probably sort these or assume that they are in order?
    for (int i = 0; i < track.nlanes; i++) {
      if (track.lanedist[i] < track.lanedist[min_lane])
        min_lane = i;
      if (track.lanedist[i] > track.lanedist[max_lane])
        max_lane = i;
    }

    if (!target_right && track.lanedist[now.endLane] > track.lanedist[min_lane])
    { std::cout << "LANELEFT:min="<<min_lane<<",max="<<max_lane<<",cur="<<now.endLane<<" nextsw="<<now.pieceIndex+next_switch<<std::endl;
      return -1; }
    else if (target_right && track.lanedist[now.endLane] < track.lanedist[max_lane])
    { std::cout << "LANERIGHT:min="<<min_lane<<",max="<<max_lane<<",cur="<<now.endLane<<" nextsw="<<now.pieceIndex+next_switch<<std::endl;
      return 1; }
  }
  return 0;
}

double game_logic::compute_throttle(const CarPosition& now) const
{
  if (mycar.nticks < Player::COEF_MEAS_TICKS)
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

game_logic::msg_vector game_logic::on_your_car(const jsoncons::json& data)
{
  mycolor = data["color"].as<std::string>();
  return { };
}
