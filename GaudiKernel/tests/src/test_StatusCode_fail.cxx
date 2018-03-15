#include "GaudiKernel/StatusCode.h"

int main() {
  {
    StatusCode sc = 42;  // FAIL01: no implicit conversion from int
    int i = sc;          // FAIL02: no implicit conversion to int
    bool b = sc;         // FAIL03: no implicit conversion to bool
    (void)i; (void)b;    // silence "unused" compiler warnings
  }
  {
    StatusCode sc1, sc2;
    StatusCode sc = sc1 && sc2;  // FAIL04: no conversion from bool
  }
}
