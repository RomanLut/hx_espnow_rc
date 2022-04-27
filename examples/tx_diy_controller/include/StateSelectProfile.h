#pragma once

#include "StateBase.h"

//=====================================================================
//=====================================================================
class StateSelectProfile: public StateBase
{
private:
    uint32_t stateTime;
    bool waitUnpress;
    bool exit;
    uint8_t profileId;

public:

    static StateSelectProfile instance;

    void SetLEDS4Profile();

    virtual void onEnter() override;
    virtual void onRun(uint32_t t) override;

};

