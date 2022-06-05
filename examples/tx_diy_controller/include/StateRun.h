#pragma once

#include "StateBase.h"

//=====================================================================
//=====================================================================
class StateRun: public StateBase
{
private:
    bool configModeNameAudioPlayed = false;
public:

    static StateRun instance;

    virtual void onEnter(StateBase *prevState) override;
    virtual void onRun(uint32_t t) override;

};

