import math; // Import our module
#include <iostream>

int main() {
  double x = 5.0;
  std::cout << "Square of " << x << " is " << square(x) << '\n';
  std::cout << "Cube of " << x << " is " << cube(x) << '\n';
  // helper(x);  // This would fail to compile - helper is not exported
  return 0;
}
