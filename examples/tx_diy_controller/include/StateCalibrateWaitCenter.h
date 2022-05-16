#pragma once

#include "StateBase.h"

//=====================================================================
//=====================================================================
class StateCalibrateWaitCenter: public StateBase
{
private:
    uint32_t stateTime;
    int soundSkipCount = 0;
public:

    static StateCalibrateWaitCenter instance;

    virtual void onEnter() override;
    virtual void onRun(uint32_t t) override;

};

