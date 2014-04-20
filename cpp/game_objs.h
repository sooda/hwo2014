#ifndef GAME_OBJS_H
#define GAME_OBJS_H

#include <iostream>
#include <jsoncons/json.hpp>
#include <vector>

struct Piece {
public:
  double length; // straight
  double radius, angle; // bend
  bool switch_;
};

std::ostream& operator<<(std::ostream& os, Piece p);

struct Track {
  std::vector<Piece> track;
  int lanes; // are their indices needed or just the count?
};

namespace jsoncons {

template <class Storage>
class value_adapter<char, Storage, Piece> {
  public:
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
    Track as(const basic_json<char, Storage>& val) const {
      auto lanes  = val["lanes"] .template as<json::array>();
      auto pieces = val["pieces"].template as<std::vector<Piece>>();
      return Track{
        pieces,
        (int)lanes.size()
      };
    }
};

}

#endif
