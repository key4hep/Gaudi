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

// Gaudi's include
#include <Gaudi/PluginService.h>

// Generated include
#include <DUMMYGAUDIDOWNSTREAMPROJECT_VERSION.h>
#include <MainVersion.h>

using namespace std;

int main(int, char const*[])
{
    cout << "I depend on Gaudi" << endl;
    if(MAIN_VERSION != DUMMYGAUDIDOWNSTREAMPROJECT_VERSION)
    {
        cerr << "Error: MAIN_VERSION (" << MAIN_VERSION <<
            ") != DUMMYGAUDIDOWNSTREAMPROJECT_VERSION (" <<
            DUMMYGAUDIDOWNSTREAMPROJECT_VERSION << endl;
        return 1;
    }
    return 0;
}
