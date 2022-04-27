#pragma once

#include "StateBase.h"

//=====================================================================
//=====================================================================
class StateInit: public StateBase
{
private:
    uint32_t stateTime;
public:

    static StateInit instance;

    virtual void onEnter() override;
    virtual void onRun(uint32_t t) override;

};

