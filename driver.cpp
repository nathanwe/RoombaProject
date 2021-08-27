#include <iostream>
#include "definitions.h"
#include "agent.h"
#include "environment.h"


int main (int argc, char* argv[]) {
  int x = 3;
  std::cout << x;
  std::cout << "test ";
  Environment e(argv[1]);

  while (!e.Done()) {
    e.Print();
    e.Step();
  }
  e.Print();
}
