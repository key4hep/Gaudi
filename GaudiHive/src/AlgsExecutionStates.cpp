#include "AlgsExecutionStates.h"

// A simple map to easily translate a state to its name
std::map<AlgsExecutionStates::State,std::string> AlgsExecutionStates::stateNames = {
    {INITIAL,"INITIAL"},
    {CONTROLREADY,"CONTROLREADY"},
    {DATAREADY,"DATAREADY"},
    {SCHEDULED,"SCHEDULED"},
    {EVTACCEPTED,"EVTACCEPTED"},
    {EVTREJECTED,"EVTREJECTED"},
    {ERROR,"ERROR"}
    };

