#ifndef GAME_OBJS_H
#define GAME_OBJS_H

#include <iostream>
#include <jsoncons/json.hpp>
#include <vector>

struct Piece {
  double length; // straight
  double radius, angle; // bend
  bool switch_;

  double travel(double fromcenter) {
    // will need lane distance from center information in bends
    // approximate for now
    if (length)
      return length;

    if (angle < 0) {
      // left turn
      angle = -angle;
      fromcenter = -fromcenter;
    }
    // fromcenter reduces the radius in right turns
    // turning left, a negative fromcenter is further away
    return 2 * 3.14159265 * (radius - fromcenter) * angle / 360;
    // how about switch pieces?
  }
};

std::ostream& operator<<(std::ostream& os, Piece p);

struct Track {
  std::vector<Piece> track;
  int lanes; // are their indices needed or just the count?
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
      auto lanes  = val["lanes"] .template as<json::array>();
      auto pieces = val["pieces"].template as<std::vector<Piece>>();
      return Track{
        pieces,
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
