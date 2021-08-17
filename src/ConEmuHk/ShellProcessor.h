﻿
/*
Copyright (c) 2011-present Maximus5
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

#include "../common/Common.h"
#include "../common/CmdLine.h"
#include "../common/MModule.h"
#include "../common/RConStartArgs.h"
#include <memory>

struct RConStartArgs;

typedef DWORD ChangeExecFlags;
const ChangeExecFlags
	CEF_NEWCON_SWITCH   = 1, // If command line has "-new_console"
	CEF_NEWCON_PREPEND  = 2, // If we need to prepend command with "-new_console" to ensure that "start cmd" will be processed properly
	CEF_DEFAULT = 0;

enum CmdOnCreateType;

enum class ShellWorkOptions : uint32_t
{
	None = 0,
	// during CreateProcessXXX the flag CREATE_SUSPENDED was already set
	WasSuspended = 0x00000001,
	// Either native or .net debugging is supposed
	WasDebug = 0x00000002,

	// gbd.exe
	GnuDebugger = 0x00000010,
	// *.vshost.exe
	VsNetHost = 0x00000020,
	// VsDebugConsole.exe
	VsDebugConsole = 0x00000040,
	// msvsmon.exe
	VsDebugger = 0x00000080,

	// Starting ChildGui
	ChildGui = 0x00000100,

	// Controls if we need to inject ConEmuHk into started executable (either original, or changed ConEmu.exe / ConEmuC.exe).
	// Modified via SetNeedInjects.
	NeedInjects = 0x00001000,

	// Controls if we have to inject ConEmuHk regardless of DefTerm settings
	// Modified via SetForceInjectOriginal.
	ForceInjectOriginal = 0x00002000,

	// Special case of injecting ConEmuHk during debugging.
	// The process is started, thread is resumed until modules are initialized,
	// after that thread is suspended again and StartDefTermHooker is called.
	PostInjectWasRequested = 0x00004000,
	
	// Controls if we need to run console server, if value is false - running fo ConEmu.exe is allowed.
	// Modified via SetConsoleMode.
	ConsoleMode = 0x00010000,

	// Special case to remove RealConsole flickering. E.g. in VisualStudio we attached to the console
	// created by ConEmuC.exe server and later we have to call FreeConsole to avoid unexpected output to ConOut.
	HiddenConsoleDetachNeed = 0x00020000,

	// Create DefTerm event and mapping before resuming the process
	InheritDefTerm = 0x00040000,

	// executable was replaced with ConEmu.exe (GUI)
	ExeReplacedGui = 0x00100000,
	// executable was replaced with ConEmuC.exe (Console)
	ExeReplacedConsole = 0x00200000,
};

ShellWorkOptions operator|=(ShellWorkOptions& e1, ShellWorkOptions e2);
ShellWorkOptions operator|(ShellWorkOptions e1, ShellWorkOptions e2);
bool operator&(ShellWorkOptions e1, ShellWorkOptions e2);

class CShellProc final
{
public:
	static bool  mb_StartingNewGuiChildTab;
	static DWORD mn_LastStartedPID;

private:
	RConStartArgs m_Args;

	UINT mn_CP; // = AreFileApisANSI() ? CP_ACP : CP_OEMCP;

	// Для конвертации параметров Ansi функций (работаем через Unicode для унификации)
	CEStr mpwsz_TempAction = nullptr; // = str2wcs(asAction, nCP);
	CEStr mpwsz_TempFile = nullptr; // = str2wcs(asFile, nCP);
	CEStr mpwsz_TempParam = nullptr; // = str2wcs(asParam, nCP);

	CEStrA mpsz_TempRetFile = nullptr;
	CEStrA mpsz_TempRetParam = nullptr;
	CEStrA mpsz_TempRetDir = nullptr;
	CEStr mpwsz_TempRetFile = nullptr;
	CEStr mpwsz_TempRetParam = nullptr;
	CEStr mpwsz_TempRetDir = nullptr;

	template<typename T>
	struct StructDeleter { // deleter
		void operator()(T* p) const
		{
			SafeFree(p);
		}
	};

	// Copies for ShellExecuteEx - we may change only our memory
	LPSHELLEXECUTEINFOA mlp_ExecInfoA = nullptr, mlp_SaveExecInfoA = nullptr;
	LPSHELLEXECUTEINFOW mlp_ExecInfoW = nullptr, mlp_SaveExecInfoW = nullptr;
	// Copies for CreateProcess
	std::unique_ptr<STARTUPINFOA, StructDeleter<STARTUPINFOA>> m_lpStartupInfoA;
	std::unique_ptr<STARTUPINFOW, StructDeleter<STARTUPINFOW>> m_lpStartupInfoW;

	// Information about starting process
	DWORD mn_ImageSubsystem = 0, mn_ImageBits = 0;
	CmdArg ms_ExeTmp;

	// Describes the request, e.g. runnings options, if we are to start gdb.exe, msvsmon.exe, etc.
	ShellWorkOptions workOptions_ = ShellWorkOptions::None;

	// during CreateProcessXXX the flag CREATE_SUSPENDED was already set
	void SetWasSuspended();
	// DEBUG_ONLY_THIS_PROCESS|DEBUG_PROCESS
	void SetWasDebug();
	// gbd.exe
	void SetGnuDebugger();
	// *.vshost.exe
	void SetVsNetHost();
	// VsDebugConsole.exe
	void SetVsDebugConsole();
	// msvsmon.exe
	void SetVsDebugger();
	// Starting ChildGui
	void SetChildGui();
	void ClearChildGui();
	// Controls if we need to inject ConEmuHk into started executable (either original, or changed ConEmu.exe / ConEmuC.exe)
	void SetNeedInjects(bool value);
	// Controls if we have to inject ConEmuHk regardless of DefTerm settings
	void SetForceInjectOriginal(bool value);
	// Controls if we need to call StartDefTermHooker after resuming started process
	void SetPostInjectWasRequested();
	// Controls if we need to run console server, if value is false - running fo ConEmu.exe is allowed
	void SetConsoleMode(bool value);
	// Controls if we need to call FreeConsole after process creation
	void SetHiddenConsoleDetachNeed();
	// Controls if we need to create DefTerm event and mapping before resuming the process
	void SetInheritDefTerm();
	// Updates workOptions_ ExeReplacedGui or ExeReplacedConsole
	void SetExeReplaced(bool ourGuiExe);

	// ConEmuHooks=OFF
	bool mb_Opt_DontInject = false;
	// ConEmuHooks=NOARG
	bool mb_Opt_SkipNewConsole = false;
	// ConEmuHooks=NOSTART
	bool mb_Opt_SkipCmdStart = false;

	void CheckHooksDisabled();
	static bool GetStartingExeName(LPCWSTR asFile, LPCWSTR asParam, CEStr& rsExeTmp);

	BOOL mb_isCurrentGuiClient = FALSE;
	void CheckIsCurrentGuiClient();

	//static int mn_InShellExecuteEx;
	BOOL mb_InShellExecuteEx = FALSE;

	CESERVER_CONSOLE_MAPPING_HDR m_SrvMapping = {};

	HWND mh_PreConEmuWnd = nullptr, mh_PreConEmuWndDC = nullptr;
	BOOL mb_TempConEmuWnd = FALSE;

	// Contains ConEmu GUI PID if it's running in inside mode (e.g. in VisualStudio pane)
	DWORD deftermConEmuInsidePid_ = 0;

	enum class PrepareExecuteResult
	{
		// Restrict execution, leads to ERROR_FILE_NOT_FOUND
		Restrict = -1,
		// Bypass to WinAPI without modifications
		Bypass = 0,
		// Strings or flags were modified
		Modified = 1,
	};

	// Parameters stored and passed by during CreateProcess
	struct CreatePrepareData
	{
		// true if process is created without console (redirected IO or detached window)
		bool consoleNoWindow;
		// originally requested wShowCmd
		DWORD defaultShowCmd;
		// on prepare contains defaultShowCmd, on result contains desired wShowCmd
		DWORD showCmd;
	};

private:
	static CEStr str2wcs(const char* psz, UINT anCP);
	static CEStrA wcs2str(const wchar_t* pwsz, UINT anCP);
	bool IsAnsiConLoader(LPCWSTR asFile, LPCWSTR asParam);
	static bool PrepareNewConsoleInFile(
				CmdOnCreateType aCmd, LPCWSTR& asFile, LPCWSTR& asParam,
				CEStr& lsReplaceFile, CEStr& lsReplaceParm, CEStr& exeName);
	bool CheckForDefaultTerminal(CmdOnCreateType aCmd, LPCWSTR asAction, const DWORD* anShellFlags,
				const DWORD* anCreateFlags, const DWORD* anShowCmd);
	void CheckForExeName(const CEStr& exeName, const DWORD* anCreateFlags);
	PrepareExecuteResult PrepareExecuteParams(
				enum CmdOnCreateType aCmd,
				LPCWSTR asAction, LPCWSTR asFile, LPCWSTR asParam, LPCWSTR asDir,
				DWORD* anShellFlags, DWORD* anCreateFlags, DWORD* anStartFlags, DWORD* anShowCmd, // или Shell & Create флаги
				HANDLE* lphStdIn, HANDLE* lphStdOut, HANDLE* lphStdErr,
				CEStr& psFile, CEStr& psParam, CEStr& psStartDir);
	BOOL ChangeExecuteParams(enum CmdOnCreateType aCmd,
				LPCWSTR asFile, LPCWSTR asParam,
				ChangeExecFlags Flags, const RConStartArgs& args,
		        CEStr& psFile, CEStr& psParam);
	BOOL FixShellArgs(DWORD afMask, HWND ahWnd, DWORD* pfMask, HWND* phWnd) const;
	HWND FindCheckConEmuWindow();
	void LogExitLine(int rc, int line) const;
	void LogShellString(LPCWSTR asMessage) const;
	void RunInjectHooks(LPCWSTR asFrom, PROCESS_INFORMATION *lpPI) const;
	CreatePrepareData OnCreateProcessPrepare(const DWORD* anCreationFlags, DWORD dwFlags, WORD wShowWindow, DWORD dwX, DWORD dwY);
	bool OnCreateProcessResult(PrepareExecuteResult prepareResult, const CreatePrepareData& state, DWORD* anCreationFlags, WORD& siShowWindow, DWORD& siFlags);
	DWORD GetComspecBitness() const;

	// Validates either ghConEmuWndDC or IsDefTermEnabled
	static bool IsInterceptionEnabled();

public:
	CESERVER_REQ* NewCmdOnCreate(CmdOnCreateType aCmd,
				LPCWSTR asAction, LPCWSTR asFile, LPCWSTR asParam, LPCWSTR asDir,
				DWORD* anShellFlags, DWORD* anCreateFlags, DWORD* anStartFlags, DWORD* anShowCmd,
				int nImageBits, int nImageSubsystem,
				HANDLE hStdIn, HANDLE hStdOut, HANDLE hStdErr
				/*wchar_t (&szBaseDir)[MAX_PATH+2], BOOL& bDosBoxAllowed*/);
	BOOL LoadSrvMapping(BOOL bLightCheck = FALSE);
	BOOL GetLogLibraries() const;
	const RConStartArgs* GetArgs() const;
public:
	CShellProc();
	~CShellProc();

	CShellProc(const CShellProc&) = delete;
	CShellProc(CShellProc&&) = delete;
	CShellProc& operator=(const CShellProc&) = delete;
	CShellProc& operator=(CShellProc&&) = delete;
public:
	// Functions return TRUE when the command is allowed to be executed
	BOOL OnShellExecuteA(LPCSTR* asAction, LPCSTR* asFile, LPCSTR* asParam, LPCSTR* asDir, DWORD* anFlags, DWORD* anShowCmd);
	BOOL OnShellExecuteW(LPCWSTR* asAction, LPCWSTR* asFile, LPCWSTR* asParam, LPCWSTR* asDir, DWORD* anFlags, DWORD* anShowCmd);
	BOOL OnShellExecuteExA(LPSHELLEXECUTEINFOA* lpExecInfo);
	BOOL OnShellExecuteExW(LPSHELLEXECUTEINFOW* lpExecInfo);
	BOOL OnCreateProcessA(LPCSTR* asFile, LPCSTR* asCmdLine, LPCSTR* asDir, DWORD* anCreationFlags, LPSTARTUPINFOA* ppStartupInfo);
	BOOL OnCreateProcessW(LPCWSTR* asFile, LPCWSTR* asCmdLine, LPCWSTR* asDir, DWORD* anCreationFlags, LPSTARTUPINFOW* ppStartupInfo);
	// Called after successful process creation
	void OnCreateProcessFinished(BOOL abSucceeded, PROCESS_INFORMATION *lpPI);
	void OnShellFinished(BOOL abSucceeded, HINSTANCE ahInstApp, HANDLE ahProcess);
	// Used with DefTerm+VSDebugger
	// The hThread is resumed until modules are initialized,
	// that hThread is suspended again, StartDefTermHooker is called and finally hThread is resumed again.
	static bool OnResumeDebuggeeThreadCalled(HANDLE hThread, PROCESS_INFORMATION* lpPI = nullptr);
protected:
	static PROCESS_INFORMATION m_WaitDebugVsThread;
public:
	// Helper
	bool GetLinkProperties(LPCWSTR asLnkFile, CEStr& rsExe, CEStr& rsArgs, CEStr& rsWorkDir);
	bool InitOle32();
protected:
	MModule hOle32{};
	typedef HRESULT (WINAPI* CoInitializeEx_t)(LPVOID pvReserved, DWORD dwCoInit);
	typedef HRESULT (WINAPI* CoCreateInstance_t)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);
	CoInitializeEx_t CoInitializeEx_f = nullptr;
	CoCreateInstance_t CoCreateInstance_f = nullptr;
};

// Service functions
typedef DWORD (WINAPI* GetProcessId_t)(HANDLE Process);
extern GetProcessId_t gfGetProcessId;
