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
