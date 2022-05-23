#pragma once

#include "StateBase.h"

//=====================================================================
//=====================================================================
class StateCalibrateMinMax: public StateBase
{
private:
    uint32_t stateTime;
public:

    static StateCalibrateMinMax instance;

    virtual void onEnter(StateBase *prevState) override;
    virtual void onRun(uint32_t t) override;

};

