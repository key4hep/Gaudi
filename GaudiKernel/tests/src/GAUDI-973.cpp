#include <GaudiKernel/StringKey.h>

#include <iostream>
#include <unordered_map>

int main()
{
  std::unordered_map<Gaudi::StringKey, int> m = {{"one", 1}, {"two", 2}};

  std::cout << "one -> " << m["one"] << std::endl;
  std::cout << "two -> " << m["two"] << std::endl;

  return 0;
}
