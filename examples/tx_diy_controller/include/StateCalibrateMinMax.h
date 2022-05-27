#pragma once

#include "StateBase.h"

//=====================================================================
//=====================================================================
class StateCalibrateMinMax: public StateBase
{
private:
    uint32_t stateTime;
    uint32_t count = 0;
public:

    static StateCalibrateMinMax instance;

    virtual void onEnter(StateBase *prevState) override;
    virtual void onRun(uint32_t t) override;

};

