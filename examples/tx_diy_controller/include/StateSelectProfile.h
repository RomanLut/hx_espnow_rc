#pragma once

#include "StateBase.h"

//=====================================================================
//=====================================================================
class StateSelectProfile: public StateBase
{
private:
    uint32_t stateTime;
    bool waitUnpress;
    bool runningLight;
    bool exit;
    uint8_t profileIndex;

public:

    static StateSelectProfile instance;

    virtual void onEnter(StateBase *prevState) override;
    virtual void onRun(uint32_t t) override;

};

