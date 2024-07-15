#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "defs.h"

class State_machine{

    public:
        int32_t checkState(float, float);
};

int32_t State_machine::checkState(float, float){
    return PRE_FLIGHT;
}

#endif
