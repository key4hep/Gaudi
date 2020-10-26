#include <iostream>

#include <baz.hpp>
#include <FOOBAR_VERSION.h> // test the generated headers
#include <bazVersion.h>

using std::cerr;
using std::endl;

int main(int, char**)
{
    if(FOOBAR_MAJOR_VERSION != 0)
    {
        cerr << "FOOBAR_MAJOR_VERSION not defined" << endl;
        return 1;
    }
    if(FOOBAR_MINOR_VERSION != 2)
    {
        cerr << "FOOBAR_MINOR_VERSION not defined" << endl;
        return 2;
    }
    if(FOOBAR_MAJOR_VERSION != BAZ_MAJOR_VERSION)
    {
        cerr << "FOOBAR_MAJOR_VERSION not equal to BAZ_MAJOR_VERSION" << endl;
        return 3;
    }
    if(FOOBAR_MINOR_VERSION != BAZ_MINOR_VERSION)
    {
        cerr << "FOOBAR_MINOR_VERSION not equal to BAZ_MINOR_VERSION" << endl;
        return 4;
    }
    return 0;
}
