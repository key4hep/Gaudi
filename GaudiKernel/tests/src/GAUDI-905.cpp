#include <GaudiKernel/StringKey.h>

void func( const Gaudi::StringKey& ) {}

int main() {
  func( "test" );
  return 0;
}
