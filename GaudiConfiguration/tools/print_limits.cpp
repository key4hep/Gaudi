#include <cxxabi.h>
#include <iostream>
#include <limits>

template <typename T>
void print_limits_for() {
  using std::cout;
  int  status;
  auto realname = std::unique_ptr<char, decltype( free )*>(
      abi::__cxa_demangle( typeid( T ).name(), nullptr, nullptr, &status ), free );
  // the unary '+' is explained in https://stackoverflow.com/a/28414758
  cout << "    '" << realname.get() << "': (" << +std::numeric_limits<T>::min() << ", "
       << +std::numeric_limits<T>::max() << "),\n";
}

void print_limits() {
  using std::cout;
  cout << "{\n";
  print_limits_for<signed char>();
  print_limits_for<short>();
  print_limits_for<int>();
  print_limits_for<long>();
  print_limits_for<long long>();
  print_limits_for<unsigned char>();
  print_limits_for<unsigned short>();
  print_limits_for<unsigned int>();
  print_limits_for<unsigned long>();
  print_limits_for<unsigned long long>();
  cout << "}\n";
}
