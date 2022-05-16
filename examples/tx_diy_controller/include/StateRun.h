#pragma once

#include "StateBase.h"

//=====================================================================
//=====================================================================
class StateRun: public StateBase
{
private:
    bool configAudioOnce = true;
public:

    static StateRun instance;

    virtual void onEnter() override;
    virtual void onRun(uint32_t t) override;

};

