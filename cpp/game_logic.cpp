#include "game_logic.h"
#include "protocol.h"

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
    }
{
}

game_logic::msg_vector game_logic::react(const jsoncons::json& msg)
{
  const auto& msg_type = msg["msgType"].as<std::string>();
  const auto& data = msg["data"];
  auto action_it = action_map.find(msg_type);
  if (action_it != action_map.end())
  {
    return (action_it->second)(this, data);
  }
  else
  {
    std::cout << "Unknown message type: " << msg_type << std::endl;
    return { make_ping() };
  }
}

game_logic::msg_vector game_logic::on_join(const jsoncons::json& data)
{
  std::cout << "Joined" << std::endl;
  return { make_ping() };
}

game_logic::msg_vector game_logic::on_game_init(const jsoncons::json& data)
{
  std::cout << "Game init" << std::endl;

  track = data["race"]["track"].as<Track>();
  for (auto& piece: track.track) {
    std::cout << piece << std::endl;
  }
  std::cout << track.track.size() << " pieces in total, lane count " << track.lanes << std::endl;

  auto cars = data["race"]["cars"];
  for (size_t i = 0; i < cars.size(); i++) {
    std::cout << "car "
      << cars[i]["id"]["name"] << " "
      << cars[i]["id"]["color"]
      << std::endl;
  }

  mycar = { {"","",0.0,0,0.0,0,0}, 0.0, 0 };

  return { make_ping() };
}

game_logic::msg_vector game_logic::on_game_start(const jsoncons::json& data)
{
  std::cout << "Race started" << std::endl;
  return { make_ping() };
}

game_logic::msg_vector game_logic::on_car_positions(const jsoncons::json& data)
{
  std::cout << "Position tick" << std::endl;

  auto positions = data.as<std::vector<CarPosition>>();
  for (CarPosition& p: positions) {
    if (&p != &positions.front())
      std::cout << ", ";
    std::cout << "(" << p.pieceIndex << "," << p.inPieceDistance << ")";
  }
  std::cout << std::endl;

  CarPosition& now = positions[0]; // XXX parse my color

  double travel;
  if (now.pieceIndex == mycar.prev.pieceIndex) {
    travel = now.inPieceDistance - mycar.prev.inPieceDistance;
  } else {
    // changed piece between ticks
    double last_remaining = track.track[mycar.prev.pieceIndex].travel(-10) - mycar.prev.inPieceDistance;
    double in_this = now.inPieceDistance;
    travel = last_remaining + in_this;
  }
  double ticktime = 1.0 / 60;
  double speed = travel / ticktime;
  mycar.tottravel += travel;

  std::cout
    << "ticks " << mycar.nticks
    << " current piece " << now.pieceIndex
    << " total traveled: " << mycar.tottravel
    << ", this travel: " << travel
    << " speed: " << speed << std::endl;

  std::cerr << mycar.nticks << " " << mycar.tottravel << " " << travel << " " << speed << std::endl;

  mycar.prev = now;
  mycar.nticks++;

  return { make_throttle(0.6) };
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
