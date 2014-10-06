#include <GaudiKernel/StringKey.h>

void func(const Gaudi::StringKey& k) {}

int main() {
  func("test");
  return 0;
}
