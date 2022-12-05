#include <application.hpp>

int main() {
  pusn::application sim;
  sim.init("Movement Interpolation");
  sim.main_loop();
  return 0;
}
