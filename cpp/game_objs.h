#ifndef GAME_OBJS_H
#define GAME_OBJS_H

#include <iostream>
#include <jsoncons/json.hpp>
#include <vector>
#include <array>

struct Piece {
  double length; // straight
  double radius, angle; // bend
  bool switch_;

  double travel(double fromcenter) const {
    if (length) {
      // straight
      return length;
    } else if (angle >= 0) {
      // right turn
      // a positive value is "to the right", i.e. shortens the travel
      return 2 * 3.14159265 * (radius - fromcenter) * angle / 360;
    } else {
      // left turn
      // positive value increases the travel here as it's the outer lane
      return 2 * 3.14159265 * (radius + fromcenter) * -angle / 360;
    }

    // how about switch pieces?
  }
};

std::ostream& operator<<(std::ostream& os, Piece p);

struct Track {
  std::vector<Piece> track;
  std::array<double, 4> lanedist; // distance from center
  int nlanes;
};

struct CarPosition {
  std::string name, color;
  double angle;
  int pieceIndex;
  double inPieceDistance;
  int startLane, endLane;
};

namespace jsoncons {

template <class Storage>
class value_adapter<char, Storage, Piece> {
  public:
    Track is(const basic_json<char, Storage>& val) const {
      return true;
    }
    Piece as(const basic_json<char, Storage>& val) const {
      return Piece{
        val.get("length", 0.0)  .template as<double>(),
        val.get("radius", 0.0)  .template as<double>(),
        val.get("angle", 0.0)   .template as<double>(),
        val.get("switch", false).template as<bool>()
      };
    }
};

template <class Storage>
class value_adapter<char, Storage, Track> {
  public:
    Track is(const basic_json<char, Storage>& val) const {
      return true;
    }
    Track as(const basic_json<char, Storage>& val) const {
      auto pieces = val["pieces"].template as<std::vector<Piece>>();
      auto lanes  = val["lanes"] .template as<json::array>();

      std::array<double, 4> dists = {0.0};
      // should be 1..4 lanes but min() to be safe
      for (size_t i = 0; i < std::min(lanes.size(), 4UL); i++)
        dists[lanes[i]["index"].template as<int>()] = lanes[i]["distanceFromCenter"].template as<double>();

      return Track{
        pieces,
        dists,
        (int)lanes.size()
      };
    }
};

template <class Storage>
class value_adapter<char, Storage, CarPosition> {
  public:
    CarPosition as(const basic_json<char, Storage>& val) const {
      auto pos = val["piecePosition"];
      return CarPosition{
        val["id"]["name"]            .template as<std::string>(),
        val["id"]["color"]           .template as<std::string>(),
        val["angle"]                 .template as<double>(),
        pos["pieceIndex"]            .template as<int>(),
        pos["inPieceDistance"]       .template as<double>(),
        pos["lane"]["startLaneIndex"].template as<int>(),
        pos["lane"]["endLaneIndex"]  .template as<int>(),
      };
    }
};

}

#endif
