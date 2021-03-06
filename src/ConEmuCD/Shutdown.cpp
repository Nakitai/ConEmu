
/*
Copyright (c) 2018-present Maximus5
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#pragma once

#include "../common/defines.h"
#include "Shutdown.h"

namespace Shutdown
{

/*
using ShutdownCallback = void (*)(LPARAM);
struct ShutdownEvent
{
	ShutdownCallback callback = nullptr;
	LPARAM lParam = 0;
	ShutdownEvent* next = nullptr;
};
*/

static ShutdownEvent* events = nullptr;

void RegisterEvent(ShutdownCallback callback, LPARAM lParam)
{
	ShutdownEvent* new_event = new ShutdownEvent();
	new_event->callback = callback;
	new_event->lParam = lParam;
	new_event->next = (ShutdownEvent*)InterlockedExchangePointer((PVOID*)&events, new_event);
}
void ProcessShutdown()
{
	ShutdownEvent* cur_event = (ShutdownEvent*)InterlockedExchangePointer((PVOID*)&events, nullptr);
	while (cur_event)
	{
		if (cur_event->callback)
			cur_event->callback(cur_event->lParam);
		_ASSERTE(cur_event != cur_event->next);
		cur_event = cur_event->next;
	}
}

};
