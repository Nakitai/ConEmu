
/*
Copyright (c) 2009-present Maximus5
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

#include "defines.h"
#include "MConHandle.h"

#include <intrin.h>

WARNING("После перехода на альтернативный сервер - должен работать строго в 'стандартном' режиме (mn_StdMode=STD_OUTPUT_HANDLE/STD_INPUT_HANDLE)");

MConHandle::MConHandle(LPCWSTR asName)
	: mcs_Handle(true)
{
	m_sec.nLength = sizeof(m_sec);
	m_sec.bInheritHandle = TRUE;
	
	lstrcpynW(ms_Name, asName ? asName : L"", 9);
	memset(m_log, 0, sizeof(m_log));
	/*
	FAR2 последний
	Conemu последний

	без плагов каких либо

	пропишем одну ассоциацию

	[HKEY_CURRENT_USER\Software\Far2\Associations\Type0]
	"Mask"="*.ini"
	"Description"=""
	"Execute"="@"
	"AltExec"=""
	"View"=""
	"AltView"=""
	"Edit"=""
	"AltEdit"=""
	"State"=dword:0000003f

	ФАР валится по двойному щелчку на INI файле. По Enter - не валится.


	1:31:11.647 Mouse: {10x15} Btns:{L} KeyState: 0x00000000 |DOUBLE_CLICK
	CECMD_CMDSTARTED(Cols=80, Rows=25, Buf=1000, Top=-1)
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	SyncWindowToConsole(Cols=80, Rows=22)
	Current size: Cols=80, Buf=1000
	CECMD_CMDFINISHED(Cols=80, Rows=22, Buf=0, Top=-1)
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	1:31:11.878 Mouse: {10x15} Btns:{} KeyState: 0x00000000
	Current size: Cols=80, Rows=22
	1:31:11.906 Mouse: {10x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:11.949 Mouse: {10x15} Btns:{L} KeyState: 0x00000000 |DOUBLE_CLICK
	CECMD_CMDSTARTED(Cols=80, Rows=22, Buf=1000, Top=-1)
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	Current size: Cols=80, Buf=1000
	CECMD_CMDFINISHED(Cols=80, Rows=22, Buf=0, Top=-1)
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	1:31:12.163 Mouse: {10x15} Btns:{} KeyState: 0x00000000
	1:31:12.196 Mouse: {10x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	Current size: Cols=80, Rows=22
	1:31:12.532 Mouse: {11x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.545 Mouse: {13x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.573 Mouse: {14x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.686 Mouse: {15x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.779 Mouse: {16x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.859 Mouse: {16x15} Btns:{L} KeyState: 0x00000000
	1:31:12.876 Mouse: {16x15} Btns:{L} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:12.944 Mouse: {16x15} Btns:{} KeyState: 0x00000000
	1:31:12.956 Mouse: {16x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:13.010 Mouse: {16x15} Btns:{L} KeyState: 0x00000000 |DOUBLE_CLICK
	CECMD_CMDSTARTED(Cols=80, Rows=22, Buf=1000, Top=-1)
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	SyncWindowToConsole(Cols=80, Rows=19)
	Current size: Cols=80, Buf=1000
	CECMD_CMDFINISHED(Cols=80, Rows=19, Buf=0, Top=-1)
	1:31:13.150 Mouse: {16x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:13.175 Mouse: {16x15} Btns:{L} KeyState: 0x00000000
	1:31:13.206 Mouse: {16x15} Btns:{L} KeyState: 0x00000000 |MOUSE_MOVED
	SetConsoleSizeSrv.Not waiting for ApplyFinished
	1:31:13.240 Mouse: {16x15} Btns:{} KeyState: 0x00000000
	Current size: Cols=80, Rows=191:31:13.317 Mouse: {16x15} Btns:{} KeyState: 0x00000000 |MOUSE_MOVED
	1:31:13.357 Mouse: {16x15} Btns:{L} KeyState: 0x00000000 |DOUBLE_CLICK


	1:31:11 CurSize={80x25} ChangeTo={80x25} RefreshThread :CECMD_SETSIZESYNC
	1:31:11 CurSize={80x1000} ChangeTo={80x25} RefreshThread :CECMD_SETSIZESYNC
	1:31:11 CurSize={80x25} ChangeTo={80x25} RefreshThread :CECMD_SETSIZESYNC
	1:31:11 CurSize={80x1000} ChangeTo={80x22} RefreshThread :CECMD_SETSIZESYNC
	1:31:12 CurSize={80x22} ChangeTo={80x22} RefreshThread :CECMD_SETSIZESYNC
	1:31:12 CurSize={80x1000} ChangeTo={80x22} RefreshThread :CECMD_SETSIZESYNC
	1:31:13 CurSize={80x22} ChangeTo={80x22} RefreshThread :CECMD_SETSIZESYNC
	1:31:13 CurSize={80x1000} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:15 CurSize={80x19} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:15 CurSize={80x1000} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:16 CurSize={80x19} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:16 CurSize={80x1000} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:16 CurSize={80x19} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:16 CurSize={80x19} ChangeTo={80x19} RefreshThread :CECMD_SETSIZESYNC
	1:31:16 CurSize={80x1000} ChangeTo={80x16} RefreshThread :CECMD_SETSIZESYNC
	1:31:25 CurSize={80x16} ChangeTo={80x16} RefreshThread :CECMD_SETSIZESYNC
	1:31:25 CurSize={80x1000} ChangeTo={80x16} RefreshThread :CECMD_SETSIZESYNC




	*/
	//OSVERSIONINFO osv = {sizeof(OSVERSIONINFO)}; GetVersionEx(&osv);
	//if (osv.dwMajorVersion == 6 && osv.dwMinorVersion == 1) {
	//	if (!lstrcmpW(ms_Name, L"CONOUT$"))
	//		mn_StdMode = STD_OUTPUT_HANDLE;
	//	else if (!lstrcmpW(ms_Name, L"CONIN$"))
	//		mn_StdMode = STD_INPUT_HANDLE;
	//}
}

MConHandle::~MConHandle()
{
	Close();
}

// ReSharper disable once CppParameterMayBeConst
void MConHandle::LogHandle(UINT evt, HANDLE h)
{
	const LONG i = (_InterlockedIncrement(&m_logIdx) & (HANDLE_BUFFER_SIZE - 1));
	m_log[i].evt = static_cast<Event::EventType>(evt);
	DEBUGTEST(m_log[i].time = GetTickCount());
	m_log[i].TID = GetCurrentThreadId();
	m_log[i].h = h;
}

// may point to some GLOBAL SCOPE variable with console handle
void MConHandle::SetHandlePtr(HANDLE* ppOutBuffer)
{
	mpp_OutBuffer = ppOutBuffer;
}

// may point to some GLOBAL SCOPE variable with console handle
void MConHandle::SetHandlePtr(const MConHandle& out_buffer)
{
	mpp_OutBuffer = &out_buffer.mh_Handle;
}

// ReSharper disable once CppParameterMayBeConst
bool MConHandle::SetHandle(HANDLE newHandle, const StdMode stdMode)
{
	MSectionLockSimple CS; CS.Lock(&mcs_Handle);
	// ReSharper disable once CppLocalVariableMayBeConst
	HANDLE h = (newHandle == nullptr) ? INVALID_HANDLE_VALUE : newHandle;
	Close();
	mh_Handle = h;
	mn_StdMode = stdMode;
	return HasHandle();
}

bool MConHandle::HasHandle() const
{
	return mh_Handle && (mh_Handle != INVALID_HANDLE_VALUE);
}

MConHandle::operator const HANDLE()
{
	return GetHandle();
}

HANDLE MConHandle::Release()
{
	MSectionLockSimple CS; CS.Lock(&mcs_Handle);
	HANDLE h = INVALID_HANDLE_VALUE;
	std::swap(h, mh_Handle);
	mn_StdMode = StdMode::None;
	return h;
}

HANDLE MConHandle::GetHandle()
{
	if (mpp_OutBuffer && *mpp_OutBuffer && (*mpp_OutBuffer != INVALID_HANDLE_VALUE))
	{
		LogHandle(Event::e_GetHandlePtr, *mpp_OutBuffer);
		return *mpp_OutBuffer;
	}

	if (mh_Handle == INVALID_HANDLE_VALUE)
	{
		if (mn_StdMode != StdMode::None)
		{
			mh_Handle = GetStdHandle(mn_StdMode == StdMode::Input ? STD_INPUT_HANDLE : STD_OUTPUT_HANDLE);
			LogHandle(Event::e_CreateHandleStd, mh_Handle);
		}
		else
		{
			// Чтобы случайно не открыть хэндл несколько раз в разных потоках
			MSectionLockSimple CS; CS.Lock(&mcs_Handle);

			// Во время ожидания хэндл мог быт открыт в другом потоке
			if (mh_Handle == INVALID_HANDLE_VALUE)
			{
				mh_Handle = CreateFileW(ms_Name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
										&m_sec, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (!mh_Handle) // Is not expected, JIC
					mh_Handle = INVALID_HANDLE_VALUE;

				if (mh_Handle != INVALID_HANDLE_VALUE)
				{
					mb_OpenFailed = FALSE;
				}
				else
				{
					mn_LastError = GetLastError();

					if (!mb_OpenFailed)
					{
						mb_OpenFailed = TRUE; // чтобы ошибка вываливалась только один раз!
						char szErrMsg[512], szNameA[10], szSelfFull[MAX_PATH];
						const char *pszSelf;
						// ReSharper disable once CppJoinDeclarationAndAssignment
						char *pszDot;

						if (!GetModuleFileNameA(nullptr, szSelfFull, MAX_PATH))
						{
							pszSelf = "???";
						}
						else
						{
							pszSelf = strrchr(szSelfFull, '\\');
							if (pszSelf) pszSelf++; else pszSelf = szSelfFull;

							// ReSharper disable once CppJoinDeclarationAndAssignment
							pszDot = strrchr(const_cast<char*>(pszSelf), '.');

							if (pszDot) *pszDot = 0;
						}

						WideCharToMultiByte(CP_OEMCP, 0, ms_Name, -1, szNameA, sizeof(szNameA), nullptr, nullptr);
						sprintf_c(szErrMsg, "%s: CreateFile(%s) failed, ErrCode=0x%08X\n", pszSelf, szNameA, mn_LastError);
						// ReSharper disable once CppLocalVariableMayBeConst
						HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

						if (h && h != INVALID_HANDLE_VALUE)
						{
							DWORD dwWritten = 0;
							WriteFile(h, szErrMsg, lstrlenA(szErrMsg), &dwWritten, nullptr);
						}
					}
				}

				LogHandle(Event::e_CreateHandle, mh_Handle);
			}
		}
	}

	LogHandle(Event::e_GetHandle, mh_Handle);
	return mh_Handle;
}

void MConHandle::Close()
{
	// Если установлен указатель на хэндл буфера - закрывать не будем
	if (mpp_OutBuffer && *mpp_OutBuffer && (*mpp_OutBuffer != INVALID_HANDLE_VALUE))
	{
		return;
	}

	if (mh_Handle != INVALID_HANDLE_VALUE)
	{
		if (mn_StdMode != StdMode::None)
		{
			LogHandle(Event::e_CloseHandleStd, mh_Handle);
			mh_Handle = INVALID_HANDLE_VALUE;
		}
		else
		{
			// ReSharper disable once CppLocalVariableMayBeConst
			HANDLE h = mh_Handle;
			LogHandle(Event::e_CloseHandleStd, h);
			mh_Handle = INVALID_HANDLE_VALUE;
			mb_OpenFailed = FALSE;
			CloseHandle(h);
		}
	}
}
