#pragma once

#include "StateBase.h"

//=====================================================================
//=====================================================================
class StateCalibrateWaitCenter: public StateBase
{
private:
    uint32_t stateTime;
    bool waitUnpress;
    int count = 0;
public:

    static StateCalibrateWaitCenter instance;

    virtual void onEnter(StateBase *prevState) override;
    virtual void onRun(uint32_t t) override;

};

