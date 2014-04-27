#include <iostream>
#include <string>
#include <jsoncons/json.hpp>
#include "protocol.h"
#include "connection.h"
#include "game_logic.h"

using namespace hwo_protocol;

void run(hwo_connection& connection, const std::string& name,
    const std::string& key, const std::string& track = "", const std::string& pwd = "", const std::string& carcount = "")
{
  game_logic game;
  if (track == "")
    connection.send_requests({ make_join(name, key) });
  else if (pwd == "")
    connection.send_requests({ make_create_single(name, key, track) });
  else
    connection.send_requests({ make_join_race(name, key, track, pwd, carcount) });

  for (;;)
  {
    boost::system::error_code error;
    auto response = connection.receive_response(error);

    if (error == boost::asio::error::eof)
    {
      std::cout << "Connection closed" << std::endl;
      break;
    }
    else if (error)
    {
      throw boost::system::system_error(error);
    }

    connection.send_requests(game.react(response));
  }
}

int main(int argc, const char* argv[])
{
  try
  {
    if (argc < 5 || argc > 8)
    {
      std::cerr << "Usage: ./run host port botname botkey [trackname] [pwdname ncars]" << std::endl;
      std::cerr << "no track/pwd: join, track only: create single, track+pwd+count: joinrace" << std::endl;
      return 1;
    }

    const std::string host(argv[1]);
    const std::string port(argv[2]);
    const std::string name(argv[3]);
    const std::string key(argv[4]);
    const std::string track(argc >= 6 ? argv[5] : "");
    const std::string pwd(argc >= 7 ? argv[6] : "");
    const std::string carcount(argc >= 8 ? argv[7] : "");
    std::cout << "Host: " << host << ", port: " << port << ", name: " << name << ", key: " << key << ", track: " << track << ", pwd: " << pwd << ", count: " << carcount << std::endl;

    hwo_connection connection(host, port, track);
    run(connection, name, key, track, pwd, carcount);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return 2;
  }

  return 0;
}
