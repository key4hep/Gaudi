// Compile with:
// g++ --shared -fPIC -o libchecker.so checker.cpp

#include <iostream>
#include <string>
extern "C"
bool check(char *s) {
  std::string data(s);
  std::cout << data << std::endl;
  return data.size() > 3;
}

/* python:
from ctypes import CDLL, c_char_p, c_bool
lib = CDLL("./libchecker.so")
check = lib.check
check.argtypes = [c_char_p]
check.restype = c_bool
check(1)
check("ciao")
*/
