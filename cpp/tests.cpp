#include "game_objs.h"
#include <iostream>

using namespace std;
using namespace jsoncons;

void obj_parse_test() {
  string src("{\"track\":{\"pieces\":[{\"length\":100.0,\"switch\":true},{\"radius\":200,\"angle\":22.5}],\"lanes\":[{\"index\":0}]}}");
  json j(json::parse_string(src));
  cout << j << endl;

  cout << "pieces:" << endl;
  vector<Piece> ps = j["track"]["pieces"].as<vector<Piece>>();
  for (auto x: ps)
    cout << x << endl;

  cout << "track:" << endl;
  Track tr = j["track"].as<Track>();
  for (auto x: tr.track)
    cout << x << endl;
  cout << tr.lanes << endl;
}

void keimola_dump() {
  Track kei = json::parse_file("keimola.json").as<Track>();
  double totlen1 = 0.0;
  double totlen2 = 0.0;
  for (Piece p: kei.track) {
    cout << p << endl;
    totlen1 += p.travel(10.0);
    totlen2 += p.travel(-10.0);
  }
  cout << "lane lenghts " << totlen1 << " and " << totlen2 << endl;
}


int main() {
  obj_parse_test();
  keimola_dump();
  return 0;
}
