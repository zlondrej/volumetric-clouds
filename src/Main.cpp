#include "Main.hpp"

int main(int argc, char **argv) {
  Main program = Main(argc, argv);
  program.run();
}


Main::Main(int c, char **v) : argc(c), argv(v) {}

void Main::run() {
  return;
}
