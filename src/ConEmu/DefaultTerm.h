﻿
/*
Copyright (c) 2012-present Maximus5
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

#define SHOWDEBUGSTR

#define DEBUGSTRDEFTERM(s) DEBUGSTR(s)

#include "../common/DefTermBase.h"
#include "../common/MFileMapping.h"

class CDefaultTerminal final : public CDefTermBase
{
public:
	CDefaultTerminal();
	~CDefaultTerminal() override;

	CDefaultTerminal(const CDefaultTerminal&) = delete;
	CDefaultTerminal(CDefaultTerminal&&) = delete;
	CDefaultTerminal& operator=(const CDefaultTerminal&) = delete;
	CDefaultTerminal& operator=(CDefaultTerminal&&) = delete;

	void StartGuiDefTerm(bool bManual, bool bNoThreading = false);
	void OnTaskbarCreated();

	void CheckRegisterOsStartup();
	void ApplyAndSave(bool bApply, bool bSaveToReg);
	static bool IsRegisteredOsStartup(CEStr* rszData, bool* pbLeaveInTSA);

	bool isDefaultTerminalAllowed(bool bDontCheckName = false) override; // !(gpConEmu->DisableSetDefTerm || !gpSet->isSetDefaultTerminal)

	void LogHookingStatus(DWORD nForePID, LPCWSTR sMessage) override;
	bool isLogging() override;

	/// @brief Create/update CONEMU_INSIDE_DEFTERM_MAPPING
	void UpdateDefTermMapping();

protected:
	CDefTermBase* GetInterface() override;
	int  DisplayLastError(LPCWSTR asLabel, DWORD dwError=0, DWORD dwMsgFlags=0, LPCWSTR asTitle=nullptr, HWND hParent=nullptr) override;
	void ShowTrayIconError(LPCWSTR asErrText) override; // Icon.ShowTrayIcon(asErrText, tsa_Default_Term);
	void ReloadSettings() override; // Copy from gpSet or load from [HKCU]
	void PreCreateThread() override;
	void PostCreateThreadFinished() override;
	void AutoClearThreads() override;
	void ConhostLocker(bool bLock, bool& bWasLocked) override;
	/// @brief Overrided by ConEmu GUI descendant to show action in the StatusBar
	/// @param processId 0 when hooking is done (remove status bar notification)
	/// @param sName is executable name or window class name
	/// @return true if StatusBar was updated (so need to reset it later)
	bool NotifyHookingStatus(DWORD processId, LPCWSTR sName) override;
	/// @brief In the Inside mode we set hooks only to our parent window which we have integrated in
	/// @param hFore GetForegroundWindow
	/// @param processId PID
	/// @return true if we may proceed with the process
	bool IsAppAllowed(HWND hFore, DWORD processId) override;

private:
	MFileMapping<CONEMU_INSIDE_DEFTERM_MAPPING> insideMapping_;
};
