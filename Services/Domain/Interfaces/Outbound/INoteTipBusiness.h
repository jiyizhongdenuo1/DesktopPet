
#pragma once

#include "DNoteCommon.h"
class INoteTipBusiness
{
public:
    INoteTipBusiness() = default;
    virtual ~INoteTipBusiness() = default;

    virtual BOOL StartTip(E_NOTE_EVENT_WAKEUP_LEVEL eLevel) = 0;

};