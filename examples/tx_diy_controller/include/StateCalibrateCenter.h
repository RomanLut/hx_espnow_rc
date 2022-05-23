#pragma once

#include "StateBase.h"

//=====================================================================
//=====================================================================
class StateCalibrateCenter: public StateBase
{
private:
    uint32_t stateTime;
public:

    static StateCalibrateCenter instance;

    virtual void onEnter(StateBase *prevState) override;
    virtual void onRun(uint32_t t) override;

};

