﻿
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

#define HIDE_USE_EXCEPTION_INFO

#ifdef _DEBUG
	#define DebugString(x) //OutputDebugString(x)
	#define DBG_XTERM(x) //CEAnsi::DebugXtermOutput(x)
	#define DBG_XTERM_MSGBOX(msg) //_ASSERTE(FALSE && msg)
#else
	#define DebugString(x) //OutputDebugString(x)
	#define DBG_XTERM(x)
	#define DBG_XTERM_MSGBOX(msg)
#endif

#include "../common/Common.h"
#include "../common/MConHandle.h"
#include "../common/MRect.h"
#include "../common/HandleKeeper.h"

#include "Ansi.h"
#include "ExtConsole.h"
#include "GuiAttach.h"
#include "hkConsoleOutput.h"
#include "hlpConsole.h"
#include "MainThread.h"
#include "DllOptions.h"
#include "../common/MHandle.h"
#include "../common/WObjects.h"

/* **************** */

extern void CheckPowerShellProgress(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);

static void CheckNeedExtScroll(ExtScrollScreenParm& scrl, const SMALL_RECT *lpScrollRectangle, const SMALL_RECT *lpClipRectangle, COORD dwDestinationOrigin)
{
	// TODO: ...
	_ASSERTE(lpClipRectangle == NULL);
	_ASSERTE(dwDestinationOrigin.X == 0);

	// Full console contents is to be scrolled
	if (!lpScrollRectangle)
		return;

	scrl.Flags |= essf_Region|essf_Global;
	scrl.Region = MakeRect(lpScrollRectangle->Left, lpScrollRectangle->Top, lpScrollRectangle->Right, lpScrollRectangle->Bottom);
}

/* **************** */

// Spare?
BOOL WINAPI OnGetConsoleMode(HANDLE hConsoleHandle,LPDWORD lpMode)
{
	//typedef BOOL (WINAPI* OnGetConsoleMode_t)(HANDLE,LPDWORD);
	ORIGINAL_KRNL(GetConsoleMode);
	BOOL b;

	b = F(GetConsoleMode)(hConsoleHandle,lpMode);

	return b;
}


BOOL WINAPI OnSetConsoleMode(HANDLE hConsoleHandle, DWORD dwMode)
{
	//typedef BOOL (WINAPI* OnSetConsoleMode_t)(HANDLE hConsoleHandle, DWORD dwMode);
	ORIGINAL_KRNL(SetConsoleMode);
	BOOL lbRc = FALSE;

	#if 0
	if (!(dwMode & ENABLE_PROCESSED_OUTPUT))
	{
		_ASSERTEX((dwMode & ENABLE_PROCESSED_OUTPUT)==ENABLE_PROCESSED_OUTPUT);
	}
	#endif

	#ifdef _DEBUG
	if (dwMode == 0)
	{
		if (HandleKeeper::IsOutputHandle(hConsoleHandle))
		{
			// Official (Win32) Vim tries to reset ConsoleMode on start, gbIsVimProcess expected to be set
			// Also occurred in cmd.exe (call from cmd.exe!_ResetConsoleMode()), will be set to `3` after this
			_ASSERTE(gbIsVimProcess || gbIsCmdProcess);
		}
	}
	#endif

	CEAnsi::WriteAnsiLogFormat("OnSetConsoleMode(0x%04x,x%02X,isVim=%s)",
		LODWORD(hConsoleHandle), dwMode, gbIsVimProcess ? "true" : "false");

	if (gbIsVimProcess)
	{
		if ((dwMode & (ENABLE_WRAP_AT_EOL_OUTPUT|ENABLE_PROCESSED_OUTPUT)) != (ENABLE_WRAP_AT_EOL_OUTPUT|ENABLE_PROCESSED_OUTPUT))
		{
			if (HandleKeeper::IsOutputHandle(hConsoleHandle))
			{
				dwMode |= ENABLE_WRAP_AT_EOL_OUTPUT|ENABLE_PROCESSED_OUTPUT;
			}
			else
			{
				dwMode |= ENABLE_WINDOW_INPUT;
			}
		}
	}

	// gh-629: Arrow keys not working in Bash for Windows
	// gh-1291: Support wsl.exe and ubuntu.exe
	if (IsWin10())
	{
		bool outputChecked = false, outputCheckResult = false;
		auto isOutput = [&outputChecked, &outputCheckResult, hConsoleHandle]()
		{
			if (!outputChecked)
			{
				outputCheckResult = HandleKeeper::IsOutputHandle(hConsoleHandle);
				outputChecked = true;
			}
			return outputCheckResult;
		};

		#ifdef _DEBUG
		if (!isOutput())
		{
			static DWORD prevInputMode = 0;
			if (prevInputMode != dwMode)
			{
				if ((prevInputMode & ENABLE_VIRTUAL_TERMINAL_INPUT) != (dwMode & ENABLE_VIRTUAL_TERMINAL_INPUT))
				{
					if (dwMode & ENABLE_VIRTUAL_TERMINAL_INPUT)
					{
						DBG_XTERM_MSGBOX("ENABLE_VIRTUAL_TERMINAL_INPUT On");
					}
					else
					{
						DBG_XTERM_MSGBOX("ENABLE_VIRTUAL_TERMINAL_INPUT Off");
					}
				}
				prevInputMode = dwMode;
			}
		}
		#endif

		const bool enableVirtualTerminalInput = (dwMode & ENABLE_VIRTUAL_TERMINAL_INPUT) != 0;
		if (enableVirtualTerminalInput
			|| (CEAnsi::gWasXTermModeSet[tmc_TerminalType].value == te_xterm && !enableVirtualTerminalInput))
		{
			if (!isOutput())
			{
				static void* xtermEnabledFor = nullptr;

				#ifdef _DEBUG
				const auto* hOut = GetStdHandle(STD_OUTPUT_HANDLE);
				#endif
				const bool isInput = HandleKeeper::IsInputHandle(hConsoleHandle);
				const bool allowChange = isInput
					|| (!enableVirtualTerminalInput && xtermEnabledFor == hConsoleHandle);

				if (allowChange)
				{
					DBG_XTERM(enableVirtualTerminalInput ? L"term=XTerm due ENABLE_VIRTUAL_TERMINAL_INPUT" : L"term=Win32 due !ENABLE_VIRTUAL_TERMINAL_INPUT");
					CEAnsi::ChangeTermMode(tmc_TerminalType, enableVirtualTerminalInput ? te_xterm : te_win32);

					xtermEnabledFor = enableVirtualTerminalInput ? hConsoleHandle : nullptr;
				}
			}
		}

		// don't use "else if" here! first "if" could be executed.
		const bool enableXterm = (dwMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
		if (CEAnsi::gbIsXTermOutput != enableXterm)
		{
			if (isOutput())
			{
				DBG_XTERM(enableXterm ? L"xTermOutput=ON due ENABLE_VIRTUAL_TERMINAL_PROCESSING" : L"xTermOutput=OFF due !ENABLE_VIRTUAL_TERMINAL_PROCESSING");
				DBG_XTERM(enableXterm ? L"AutoLfNl=OFF due ENABLE_VIRTUAL_TERMINAL_PROCESSING" : L"AutoLfNl=ON due !ENABLE_VIRTUAL_TERMINAL_PROCESSING");
				CEAnsi::StartXTermOutput(enableXterm);
			}
		}
		const bool autoLfNl = (dwMode & DISABLE_NEWLINE_AUTO_RETURN) == 0;
		if (CEAnsi::IsAutoLfNl() != autoLfNl)
		{
			if (isOutput())
			{
				DBG_XTERM(autoLfNl ? L"AutoLfNl=ON due DISABLE_NEWLINE_AUTO_RETURN" : L"AutoLfNl=OFF due DISABLE_NEWLINE_AUTO_RETURN");
				CEAnsi::SetAutoLfNl(autoLfNl);
			}
		}
	}

	#ifdef _DEBUG
	if ((gnExeFlags & (caf_Cygwin1|caf_Msys1|caf_Msys2))
		&& (dwMode & ENABLE_PROCESSED_INPUT)
		&& !HandleKeeper::IsOutputHandle(hConsoleHandle))
	{
		//_ASSERTE(!(dwMode & ENABLE_PROCESSED_INPUT));
		wchar_t szLog[120];
		msprintf(szLog, countof(szLog), L"\r\n\033[31;40m{PID:%u} Process is enabling ENABLE_PROCESSED_INPUT\033[m\r\n", GetCurrentProcessId());
		//WriteProcessed2(szLog, lstrlen(szLog), NULL, wps_Error);
		szLog[0] = 0;
	}
	#endif

	if (gfnSrvLogString && gbIsFarProcess)
	{
		wchar_t szLog[120];
		msprintf(szLog, std::size(szLog), L"Far.exe: SetConsoleMode(x%08X, x%08X)",
			LODWORD(hConsoleHandle), dwMode);
		gfnSrvLogString(szLog);
	}

	lbRc = F(SetConsoleMode)(hConsoleHandle, dwMode);

	return lbRc;
}


// Sets the attributes of characters written to the console screen buffer by
// the WriteFile or WriteConsole function, or echoed by the ReadFile or ReadConsole function.
// This function affects text written after the function call.
BOOL WINAPI OnSetConsoleTextAttribute(HANDLE hConsoleOutput, WORD wAttributes)
{
	//typedef BOOL (WINAPI* OnSetConsoleTextAttribute_t)(HANDLE hConsoleOutput, WORD wAttributes);
	ORIGINAL_KRNL(SetConsoleTextAttribute);

	CEAnsi::WriteAnsiLogFormat("SetConsoleTextAttribute(0x%02X)", wAttributes);

	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack)
	{
		SETARGS2(&lbRc, hConsoleOutput, wAttributes);
		ph->PreCallBack(&args);
	}

	#ifdef _DEBUG
	// We do not care if visible ChildGui is doing smth in their console
	// Process here only our native console windows
	if ((ghAttachGuiClient == NULL) && !gbAttachGuiClient && (wAttributes != 7))
	{
		wchar_t szDbgInfo[128];
		msprintf(szDbgInfo, countof(szDbgInfo), L"PID=%u, SetConsoleTextAttribute=0x%02X(%u)\n", GetCurrentProcessId(), (int)wAttributes, (int)wAttributes);
		DebugString(szDbgInfo);
	}
	#endif

	lbRc = F(SetConsoleTextAttribute)(hConsoleOutput, wAttributes);

	if (ph && ph->PostCallBack)
	{
		SETARGS2(&lbRc, hConsoleOutput, wAttributes);
		ph->PostCallBack(&args);
	}

	return lbRc;
}


TODO("Call UpdateAppMapRows everywhere where direct write operations are executed");

BOOL WINAPI OnWriteConsoleOutputA(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion)
{
	//typedef BOOL (WINAPI* OnWriteConsoleOutputA_t)(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
	ORIGINAL_KRNL(WriteConsoleOutputA);
	BOOL lbRc = FALSE;

	#ifdef _DEBUG
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	GetConsoleMode(hConsoleOutput, &dwMode);
	#endif

	if (ph && ph->PreCallBack)
	{
		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
		ph->PreCallBack(&args);
	}

	lbRc = F(WriteConsoleOutputA)(hConsoleOutput, lpBuffer, dwBufferSize, dwBufferCoord, lpWriteRegion);

	if (ph && ph->PostCallBack)
	{
		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
		ph->PostCallBack(&args);
	}

	return lbRc;
}


BOOL WINAPI OnWriteConsoleOutputW(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion)
{
	//typedef BOOL (WINAPI* OnWriteConsoleOutputW_t)(HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpWriteRegion);
	ORIGINAL_KRNL(WriteConsoleOutputW);
	BOOL lbRc = FALSE;

	if (ph && ph->PreCallBack)
	{
		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
		ph->PreCallBack(&args);
	}

	// PowerShell AI для определения прогресса в консоли
	if (gbPowerShellMonitorProgress)
	{
		// Первичные проверки "прогресс ли это"
		if ((dwBufferSize.Y >= 5) && !dwBufferCoord.X && !dwBufferCoord.Y
			&& lpWriteRegion && !lpWriteRegion->Left && (lpWriteRegion->Right == (dwBufferSize.X - 1))
			&& lpBuffer && (lpBuffer->Char.UnicodeChar == L' '))
		{
			#ifdef _DEBUG
			MY_CONSOLE_SCREEN_BUFFER_INFOEX csbi6 = {sizeof(csbi6)};
			apiGetConsoleScreenBufferInfoEx(hConsoleOutput, &csbi6);
			#endif
			// 120720 - PS игнорирует PopupColors в консоли. Вывод прогресса всегда идет 0x3E
			//&& (!gnConsolePopupColors || (lpBuffer->Attributes == gnConsolePopupColors)))
			if (lpBuffer->Attributes == 0x3E)
			{
				CheckPowerShellProgress(hConsoleOutput, lpBuffer, dwBufferSize, dwBufferCoord, lpWriteRegion);
			}
		}
	}

	lbRc = F(WriteConsoleOutputW)(hConsoleOutput, lpBuffer, dwBufferSize, dwBufferCoord, lpWriteRegion);

	if (ph && ph->PostCallBack)
	{
		SETARGS5(&lbRc, hConsoleOutput, lpBuffer, &dwBufferSize, &dwBufferCoord, lpWriteRegion);
		ph->PostCallBack(&args);
	}

	return lbRc;
}


BOOL WINAPI OnWriteConsoleA(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved)
{
	//typedef BOOL (WINAPI* OnWriteConsoleA_t)(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved);
	ORIGINAL_KRNL(WriteConsoleA);
	BOOL lbRc = FALSE;
	CEAnsi* pObj = NULL;

	if (lpBuffer && nNumberOfCharsToWrite && hConsoleOutput && HandleKeeper::IsOutputHandle(hConsoleOutput) && ((pObj = CEAnsi::Object()) != NULL))
	{
		lbRc = pObj->OurWriteConsoleA(hConsoleOutput, (LPCSTR)lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten);
	}
	else
	{
		// WriteConsoleA must be executed on "real console" handles only, we don't care if caller admitted an error
		lbRc = F(WriteConsoleA)(hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten, lpReserved);
	}

	return lbRc;
}


BOOL WINAPI OnWriteConsoleW(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved)
{
	return CEAnsi::OurWriteConsoleW(hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten, lpReserved, false);
}


TODO("По хорошему, после WriteConsoleOutputAttributes тоже нужно делать efof_ResetExt");
// Но пока можно это проигнорировать, большинство (?) программ, использует ее в связке
// WriteConsoleOutputAttributes/WriteConsoleOutputCharacter


BOOL WINAPI OnWriteConsoleOutputCharacterA(HANDLE hConsoleOutput, LPCSTR lpCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten)
{
	//typedef BOOL (WINAPI* OnWriteConsoleOutputCharacterA_t)(HANDLE hConsoleOutput, LPCSTR lpCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten);
	ORIGINAL_KRNL(WriteConsoleOutputCharacterA);

	FIRST_ANSI_CALL((const BYTE*)lpCharacter, nLength);

	ExtFillOutputParm fll = {sizeof(fll),
		efof_Attribute|(CEAnsi::getDisplayParm().getWasSet() ? efof_Current : efof_ResetExt),
		hConsoleOutput, {}, 0, dwWriteCoord, nLength};
	ExtFillOutput(&fll);

	BOOL lbRc = F(WriteConsoleOutputCharacterA)(hConsoleOutput, lpCharacter, nLength, dwWriteCoord, lpNumberOfCharsWritten);

	return lbRc;
}


BOOL WINAPI OnWriteConsoleOutputCharacterW(HANDLE hConsoleOutput, LPCWSTR lpCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten)
{
	//typedef BOOL (WINAPI* OnWriteConsoleOutputCharacterW_t)(HANDLE hConsoleOutput, LPCWSTR lpCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten);
	ORIGINAL_KRNL(WriteConsoleOutputCharacterW);

	FIRST_ANSI_CALL((const BYTE*)lpCharacter, nLength);

	ExtFillOutputParm fll = {sizeof(fll),
		efof_Attribute|(CEAnsi::getDisplayParm().getWasSet() ? efof_Current : efof_ResetExt),
		hConsoleOutput, {}, 0, dwWriteCoord, nLength};
	ExtFillOutput(&fll);

	BOOL lbRc = F(WriteConsoleOutputCharacterW)(hConsoleOutput, lpCharacter, nLength, dwWriteCoord, lpNumberOfCharsWritten);

	return lbRc;
}


BOOL WINAPI OnFillConsoleOutputCharacterA(HANDLE hConsoleOutput, char cCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten)
{
	ORIGINAL_KRNL(FillConsoleOutputCharacterA);

	CEAnsi::WriteAnsiLogFormat("FillConsoleOutputCharacterW(x%02X,%u,{%i,%i})",
		(uint32_t)cCharacter, nLength, dwWriteCoord.X, dwWriteCoord.Y);

	const BOOL lbRc = F(FillConsoleOutputCharacterA)(hConsoleOutput, cCharacter, nLength, dwWriteCoord, lpNumberOfCharsWritten);
	return lbRc;
}

BOOL WINAPI OnFillConsoleOutputCharacterW(HANDLE hConsoleOutput, wchar_t cCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten)
{
	ORIGINAL_KRNL(FillConsoleOutputCharacterW);

	CEAnsi::WriteAnsiLogFormat("FillConsoleOutputCharacterW(x%02X,%u,{%i,%i})",
		(uint32_t)cCharacter, nLength, dwWriteCoord.X, dwWriteCoord.Y);

	const BOOL lbRc = F(FillConsoleOutputCharacterW)(hConsoleOutput, cCharacter, nLength, dwWriteCoord, lpNumberOfCharsWritten);
	return lbRc;
}

BOOL WINAPI OnFillConsoleOutputAttribute(HANDLE hConsoleOutput, WORD wAttribute, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfAttrsWritten)
{
	ORIGINAL_KRNL(FillConsoleOutputAttribute);

	CEAnsi::WriteAnsiLogFormat("FillConsoleOutputAttribute(x%02X,%u,{%i,%i})",
		wAttribute, nLength, dwWriteCoord.X, dwWriteCoord.Y);

        const ConEmu::Color fillAttr = {ConEmu::ColorFlags::None, CONFORECOLOR(wAttribute), CONBACKCOLOR(wAttribute)};
	ExtFillOutputParm fll = {sizeof(fll),
		efof_Attribute|efof_ResetExt,
		hConsoleOutput, fillAttr, 0, dwWriteCoord, nLength};
	ExtFillOutput(&fll);

	const BOOL lbRc = F(FillConsoleOutputAttribute)(hConsoleOutput, wAttribute, nLength, dwWriteCoord, lpNumberOfAttrsWritten);
	return lbRc;
}

// After "cls" executed in cmd or powershell we have to reset nLastConsoleRow stored in our AppMap
static void CheckForCls(HANDLE hConsoleOutput, const SMALL_RECT& lpScrollRectangle, COORD dwDestinationOrigin)
{
	// Must be {0,0}, 0
	if (lpScrollRectangle.Left || lpScrollRectangle.Top || dwDestinationOrigin.X)
		return;
	if (lpScrollRectangle.Bottom != -dwDestinationOrigin.Y)
		return;
	CONSOLE_SCREEN_BUFFER_INFO sbi = {};
	if (!GetConsoleScreenBufferInfoCached(hConsoleOutput, &sbi))
		return;
	if (lpScrollRectangle.Right != sbi.dwSize.X || lpScrollRectangle.Bottom != sbi.dwSize.Y)
		return;
	// That is "cls"
	UpdateAppMapRows(0, true);
}

static void LogScrollConsoleScreenBuffer(bool unicode, const SMALL_RECT *lpScrollRectangle, const SMALL_RECT *lpClipRectangle, COORD dwDestinationOrigin, const CHAR_INFO *lpFill)
{
	if (!CEAnsi::ghAnsiLogFile)
		return;
	char src_rect[30] = "null", clip_rect[30] = "null", fill[20] = "null";
	if (lpScrollRectangle)
		msprintf(src_rect, countof(src_rect), "{%i,%i}-{%i,%i}",
			lpScrollRectangle->Left, lpScrollRectangle->Top, lpScrollRectangle->Right, lpScrollRectangle->Bottom);
	if (lpClipRectangle)
		msprintf(clip_rect, countof(clip_rect), "{%i,%i}-{%i,%i}",
			lpClipRectangle->Left, lpClipRectangle->Top, lpClipRectangle->Right, lpClipRectangle->Bottom);
	if (lpFill)
		msprintf(fill, countof(fill), "attr=0x%02X char=0x%02X",
			lpFill->Attributes, unicode ? lpFill->Char.UnicodeChar : lpFill->Char.AsciiChar);
	CEAnsi::WriteAnsiLogFormat("ScrollConsoleScreenBuffer(%s -> {%i,%i} [%s] <%s>)",
		src_rect, dwDestinationOrigin.X, dwDestinationOrigin.Y, clip_rect, fill);
}

BOOL WINAPI OnScrollConsoleScreenBufferA(HANDLE hConsoleOutput, const SMALL_RECT *lpScrollRectangle, const SMALL_RECT *lpClipRectangle, COORD dwDestinationOrigin, const CHAR_INFO *lpFill)
{
	//typedef BOOL (WINAPI* OnScrollConsoleScreenBufferA_t)(HANDLE hConsoleOutput, const SMALL_RECT *lpScrollRectangle, const SMALL_RECT *lpClipRectangle, COORD dwDestinationOrigin, const CHAR_INFO *lpFill);
	ORIGINAL_KRNL(ScrollConsoleScreenBufferA);
	BOOL lbRc = FALSE;
	bool isOut = false;

	LogScrollConsoleScreenBuffer(false, lpScrollRectangle, lpClipRectangle, dwDestinationOrigin, lpFill);

	if (HandleKeeper::IsOutputHandle(hConsoleOutput))
	{
		ExtScrollScreenParm scrl = {sizeof(scrl), essf_ExtOnly, hConsoleOutput, dwDestinationOrigin.Y - lpScrollRectangle->Top};
		CheckNeedExtScroll(scrl, lpScrollRectangle, lpClipRectangle, dwDestinationOrigin);
		ExtScrollScreen(&scrl);
		isOut = true;
	}

	lbRc = F(ScrollConsoleScreenBufferA)(hConsoleOutput, lpScrollRectangle, lpClipRectangle, dwDestinationOrigin, lpFill);

	if (lbRc && isOut && !lpClipRectangle && lpScrollRectangle && lpFill && lpFill->Char.AsciiChar == ' ')
		CheckForCls(hConsoleOutput, *lpScrollRectangle, dwDestinationOrigin);

	return lbRc;
}


BOOL WINAPI OnScrollConsoleScreenBufferW(HANDLE hConsoleOutput, const SMALL_RECT *lpScrollRectangle, const SMALL_RECT *lpClipRectangle, COORD dwDestinationOrigin, const CHAR_INFO *lpFill)
{
	typedef BOOL (WINAPI* OnScrollConsoleScreenBufferW_t)(HANDLE hConsoleOutput, const SMALL_RECT *lpScrollRectangle, const SMALL_RECT *lpClipRectangle, COORD dwDestinationOrigin, const CHAR_INFO *lpFill);
	ORIGINAL_KRNL(ScrollConsoleScreenBufferW);
	BOOL lbRc = FALSE;
	bool isOut = false;

	LogScrollConsoleScreenBuffer(false, lpScrollRectangle, lpClipRectangle, dwDestinationOrigin, lpFill);

	if (HandleKeeper::IsOutputHandle(hConsoleOutput))
	{
		ExtScrollScreenParm scrl = {sizeof(scrl), essf_ExtOnly, hConsoleOutput, dwDestinationOrigin.Y - lpScrollRectangle->Top};
		CheckNeedExtScroll(scrl, lpScrollRectangle, lpClipRectangle, dwDestinationOrigin);
		ExtScrollScreen(&scrl);
		isOut = true;
	}

	//Warning: This function called from "cmd.exe /c cls" whith arguments:
	//lpScrollRectangle - full scroll buffer
	//lpClipRectangle - NULL
	//dwDestinationOrigin = {0, -9999}

	lbRc = F(ScrollConsoleScreenBufferW)(hConsoleOutput, lpScrollRectangle, lpClipRectangle, dwDestinationOrigin, lpFill);

	if (lbRc && isOut && !lpClipRectangle && lpScrollRectangle && lpFill && lpFill->Char.UnicodeChar == ' ')
		CheckForCls(hConsoleOutput, *lpScrollRectangle, dwDestinationOrigin);

	return lbRc;
}
