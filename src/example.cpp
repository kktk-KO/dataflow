
#include <iostream>
#include <dataflow/graph.hpp>

#include <chrono>
#include <thread>
#include <atomic>

int main (int argc, char ** argv) {

  using namespace dataflow;
  graph g(std::atoi(argv[1]));

  auto & a = g.add_node<int>("a", [&] (int & v) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
//     g.find_node("a")->label = std::to_string(v++);
  }, 0);
  a.label = "a";
  auto & b = g.add_node<int>("b", [&] (int & v) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
//     g.find_node("b")->label = std::to_string(v++);
  });
  b.label = "b";

  auto & c = g.add_node<int>("c", [&] (int & v) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
//     g.find_node("c")->label = std::to_string(v++);
  });
  c.label = "c";

  auto & d = g.add_node<int>("d", [&] (int & v) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
//     g.find_node("d")->label = std::to_string(v++);
  });
  d.label = "d";

  auto & e = g.add_node<int>("e", [&] (int & v) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
//     g.find_node("e")->label = std::to_string(v++);
  });
  e.label = "e";

  auto & f = g.add_node<int>("f", [&] (int & v) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
//     g.find_node("f")->label = std::to_string(v++);
  });
  f.label = "f";

  a >> b >> c >> d >> a;
  a >> e >> f >> a;

  g.graphviz_post += "  {rank = same; b; c; d;}\n";
  g.graphviz_post += "  {rank = same; e; f;}\n";

  g.fire(a);
  g.run();
  std::this_thread::sleep_for(std::chrono::seconds(20));
  g.stop();
}
