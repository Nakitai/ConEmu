
// #CONNECTOR callback function implementing read ConIn
//   * ConEmuHk should create server pipe and pass it to GUI?
//   * ConEmu should use MArray<wchar_t> instead of CEStr to make xterm conversions
//   * If `XTerm` keyboard mode is on - never post data to conhost
//   * When `XTerm` keyboard mode is off - use conhost API to forward keypressed to native applications

/*
Copyright (c) 2015-present Maximus5
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

#define WRITE_PROCESSED_STREAM_DEFINED
// enum RequestTermConnectorMode
// enum WriteProcessedStream
// struct RequestTermConnectorParm
#include "../modules/terminals/ConnectorAPI.h"

#if defined(__GNUC__)
extern "C" {
#endif
	int WINAPI RequestTermConnector(/*[IN/OUT]*/RequestTermConnectorParm* Parm);
#if defined(__GNUC__)
};
#endif

bool isConnectorStarted();
