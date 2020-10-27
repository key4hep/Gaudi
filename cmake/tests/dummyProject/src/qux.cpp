/***********************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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
