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

#include "../common/DefTermBase.h"
#include <memory>
#include <chrono>
#include <unordered_map>
#include <atomic>


#include "../common/MEvent.h"
#include "../common/MHandle.h"

class MFileLog;
class CDefTermHk;

extern CDefTermHk* gpDefTerm;

class CDefTermChildMap;

class CDefTermHk final : public CDefTermBase
{
public:
	CDefTermHk();
	~CDefTermHk() override;

	CDefTermHk(const CDefTermHk&) = delete;
	CDefTermHk(CDefTermHk&&) = delete;
	CDefTermHk& operator=(const CDefTermHk&) = delete;
	CDefTermHk& operator=(CDefTermHk&&) = delete;

	static bool InitDefTerm();
	static bool IsDefTermEnabled();
	static void DefTermLogString(LPCSTR asMessage, LPCWSTR asLabel = nullptr);
	static void DefTermLogString(LPCWSTR asMessage, LPCWSTR asLabel = nullptr);
	static bool LoadDefTermSrvMapping(CESERVER_CONSOLE_MAPPING_HDR& srvMapping);
	static size_t GetSrvAddArgs(bool bGuiArgs, bool forceInjects, CEStr& rsArgs, CEStr& rsNewCon);
	static DWORD GetConEmuInsidePid();
	/// @brief Prepare event and mapping to resume created child process (e.g. msvsmon, VsDebugConsole, etc.)
	/// @param childPid PID of the started child process, it's suspended at the moment of CreateChildMapping call
	/// @param childHandle handle of childPid with SYNCHRONIZE access
	/// @param conemuInsidePid if this PID is not 0, we are running in the DefTerm mode initiated from ConEmu inside
	static void CreateChildMapping(DWORD childPid, const MHandle& childHandle, DWORD conemuInsidePid);

	// Start the server and attach to its console
	static HWND AllocHiddenConsole(bool bTempForVS);

	void StartDefTerm();

	static DWORD WINAPI InitDefTermContinue(LPVOID ahPrevHooks);
	DWORD mn_InitDefTermContinueTID;
	HANDLE mh_InitDefTermContinueFrom;

	// Called from hooks after successful AllocConsole
	void OnAllocConsoleFinished(HWND hNewConWnd);

	bool isDefaultTerminalAllowed(bool bDontCheckName = false) override; // !(gpConEmu->DisableSetDefTerm || !gpSet->isSetDefaultTerminal)
	void StopHookers() override;
	void ReloadSettings() override; // Copy from gpSet or load from [HKCU]

protected:
	bool FindConEmuInside(DWORD& guiPid, HWND& guiHwnd);
	std::shared_ptr<CONEMU_INSIDE_DEFTERM_MAPPING> LoadInsideSettings();
	static DWORD FindInsideParentConEmuPid();
	static DWORD LoadInsideConEmuPid(const wchar_t* mapNameFormat, DWORD param);

	DWORD   StartConsoleServer(DWORD nAttachPid, bool bNewConWnd, PHANDLE phSrvProcess);

	CDefTermBase* GetInterface() override;
	int  DisplayLastError(LPCWSTR asLabel, DWORD dwError=0, DWORD dwMsgFlags=0, LPCWSTR asTitle=nullptr, HWND hParent=nullptr) override;
	void ShowTrayIconError(LPCWSTR asErrText) override; // Icon.ShowTrayIcon(asErrText, tsa_Default_Term);
	void PostCreateThreadFinished() override;

	void LogInit();
	void LogHookingStatus(DWORD nForePID, LPCWSTR sMessage) override;

private:
	HANDLE  mh_StopEvent = nullptr;
	wchar_t ms_ExeName[MAX_PATH] = L"";
	DWORD   mn_LastCheck = 0;
	MFileLog* mp_FileLog = nullptr;

	template<typename T>
	struct StructDeleter { // insideMapInfo_ deleter
		void operator()(T* p) const
		{
			SafeFree(p);
		}
	};
	
	std::chrono::steady_clock::time_point insideMapLastCheck_{};
	std::chrono::milliseconds insideMapCheckDelay_{ 1000 };
	MSectionSimple* insideLock_ = nullptr;
	std::shared_ptr<CONEMU_INSIDE_DEFTERM_MAPPING> insideMapInfo_{};
	std::atomic<DWORD> insideConEmuPid_{ 0 };
	std::atomic<HWND> insideConEmuWnd_{ nullptr };

	MSectionSimple childDataLock_{ true };
	std::shared_ptr<CDefTermChildMap> childData_;
};
