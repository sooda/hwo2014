#include "game_objs.h"

std::ostream& operator<<(std::ostream& os, Piece p) {
  if (p.length) {
    os << "[straight " << p.length;
  } else {
    os << "[bend " << p.radius << " angle " << p.angle;
  }
  if (p.switch_) os << " switch";
  os << "]";
  return os;
}
