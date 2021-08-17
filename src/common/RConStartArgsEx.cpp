﻿
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
#define SHOWDEBUGSTR

#include "defines.h"
#include "MAssert.h"
#include "Memory.h"
#include "MStrDup.h"
#include "MStrSafe.h"
#include "RConStartArgsEx.h"
#include "CmdLine.h"

#define DEBUGSTRPARSE(s) DEBUGSTR(s)


// If you add some members - don't forget them in RConStartArgs::AssignFrom!
RConStartArgsEx::RConStartArgsEx()
	: RConStartArgs()
{
}


RConStartArgsEx::~RConStartArgsEx()
{
	// Internal for GUI tab creation
	SafeFree(this->pszEnvStrings);
	SafeFree(this->pszTaskName);
}

RConStartArgsEx::RConStartArgsEx(const RConStartArgsEx& args)
{
	AssignFrom(args);
}

bool RConStartArgsEx::AssignFrom(const RConStartArgsEx& args, bool abConcat /*= false*/)
{
	if (args.pszSpecialCmd)
	{
		SafeFree(this->pszSpecialCmd);

		//_ASSERTE(args.bDetached == FALSE); -- Allowed. While duplicating root.
		this->pszSpecialCmd = lstrdup(args.pszSpecialCmd).Detach();

		if (!this->pszSpecialCmd)
			return false;
	}

	// Startup directory. In most cases it's the same as CurDir in ConEmu.exe,
	// but it could be set from the console, if we run the command via "-new_console"
	_ASSERTE(this->pszStartupDir==nullptr);

	struct CopyValues { wchar_t** ppDst; LPCWSTR pSrc; } values[] =
	{
		{&this->pszStartupDir, args.pszStartupDir},
		{&this->pszRenameTab, args.pszRenameTab},
		{&this->pszIconFile, args.pszIconFile},
		{&this->pszPalette, args.pszPalette},
		{&this->pszWallpaper, args.pszWallpaper},
		{&this->pszMntRoot, args.pszMntRoot},
		{&this->pszAnsiLog, args.pszAnsiLog},
	};

	for (auto& p : values)
	{
		if (abConcat && *p.ppDst && !p.pSrc)
			continue;

		SafeFree(*p.ppDst);
		if (p.pSrc)
		{
			*p.ppDst = lstrdup(p.pSrc).Detach();
			if (!*p.ppDst)
				return false;
		}
	}

	if (!AssignPermissionsArgs(args, abConcat))
	{
		return false;
	}

	if (!abConcat || args.BackgroundTab || args.ForegroundTab)
	{
		this->BackgroundTab = args.BackgroundTab;
		this->ForegroundTab = args.ForegroundTab;
	}
	if (!abConcat || args.NoDefaultTerm)
	{
		this->NoDefaultTerm = args.NoDefaultTerm; _ASSERTE(args.NoDefaultTerm == crb_Undefined);
	}
	if (!abConcat || args.BufHeight)
	{
		this->BufHeight = args.BufHeight;
		this->nBufHeight = args.nBufHeight;
	}
	if (!abConcat || args.eConfirmation)
		this->eConfirmation = args.eConfirmation;
	if (!abConcat || args.ForceUserDialog)
		this->ForceUserDialog = args.ForceUserDialog;
	if (!abConcat || args.InjectsDisable)
		this->InjectsDisable = args.InjectsDisable;
	if (!abConcat || args.ForceNewWindow)
		this->ForceNewWindow = args.ForceNewWindow;
	if (!abConcat || args.ForceHooksServer)
		this->ForceHooksServer = args.ForceHooksServer;
	if (!abConcat || args.LongOutputDisable)
		this->LongOutputDisable = args.LongOutputDisable;
	if (!abConcat || args.OverwriteMode)
		this->OverwriteMode = args.OverwriteMode;
	if (!abConcat || args.nPTY)
		this->nPTY = args.nPTY;

	if (!abConcat)
	{
		this->eSplit = args.eSplit;
		this->nSplitValue = args.nSplitValue;
		this->nSplitPane = args.nSplitPane;
	}

	// Environment: Internal for GUI tab creation
	SafeFree(this->pszEnvStrings);
	this->cchEnvStrings = args.cchEnvStrings;
	if (args.cchEnvStrings && args.pszEnvStrings)
	{
		const size_t cbBytes = args.cchEnvStrings * sizeof(*this->pszEnvStrings);
		this->pszEnvStrings = static_cast<wchar_t*>(malloc(cbBytes));
		if (this->pszEnvStrings)
		{
			memmove(this->pszEnvStrings, args.pszEnvStrings, cbBytes);
		}
	}
	// Task name
	SafeFree(this->pszTaskName);
	if (args.pszTaskName && *args.pszTaskName)
		this->pszTaskName = lstrdup(args.pszTaskName).Detach();

	return true;
}


bool RConStartArgsEx::AssignPermissionsArgs(const RConStartArgsEx& args, bool abConcat /*= false*/)
{
	if (!abConcat || args.HasPermissionsArgs())
	{
		this->RunAsRestricted    = args.RunAsRestricted;
		this->RunAsAdministrator = args.RunAsAdministrator;
		this->RunAsSystem        = args.RunAsSystem;
		this->RunAsNetOnly       = args.RunAsNetOnly;
	}
	else
	{
		return true;
	}

	SafeFree(this->pszUserName); //SafeFree(this->pszUserPassword);
	SafeFree(this->pszDomain);

	if (args.pszUserName)
	{
		this->pszUserName = lstrdup(args.pszUserName).Detach();
		if (args.pszDomain)
			this->pszDomain = lstrdup(args.pszDomain).Detach();
		lstrcpy(this->szUserPassword, args.szUserPassword);
		this->UseEmptyPassword = args.UseEmptyPassword;

		// -- Do NOT fail when password is empty !!!
		if (!this->pszUserName /*|| !*this->szUserPassword*/)
			return false;
	}

	return true;
}


bool RConStartArgsEx::HasPermissionsArgs() const
{
	if (RunAsAdministrator || RunAsSystem || RunAsRestricted || pszUserName || RunAsNetOnly)
		return true;
	return false;
}


CEStr RConStartArgsEx::CreateCommandLine(bool abForTasks) const
{
	CEStr result;
	size_t cchMaxLen =
				 (pszSpecialCmd ? (lstrlen(pszSpecialCmd) + 3) : 0); // the command
	cchMaxLen += (pszStartupDir ? (lstrlen(pszStartupDir) + 20) : 0); // "-new_console:d:..."
	cchMaxLen += (pszIconFile   ? (lstrlen(pszIconFile) + 20) : 0); // "-new_console:C:..."
	cchMaxLen += (pszWallpaper  ? (lstrlen(pszWallpaper) + 20) : 0); // "-new_console:W:..."
	cchMaxLen += (pszMntRoot    ? (lstrlen(pszMntRoot) + 20) : 0); // "-new_console:W:..."
	cchMaxLen += (pszAnsiLog    ? (lstrlen(pszAnsiLog) + 20) : 0); // "-new_console:L:..."
	// Some values may contain 'invalid' symbols (like '<', '>' and so on). They will be escaped. Thats why "len*2".
	cchMaxLen += (pszRenameTab  ? (lstrlen(pszRenameTab)*2 + 20) : 0); // "-new_console:t:..."
	cchMaxLen += (pszPalette    ? (lstrlen(pszPalette)*2 + 20) : 0); // "-new_console:P:..."
	cchMaxLen += 15;
	if (RunAsAdministrator == crb_On) cchMaxLen++; // -new_console:a
	if (RunAsSystem == crb_On) cchMaxLen++; // -new_console:A
	if (RunAsRestricted == crb_On) cchMaxLen++; // -new_console:r
	if (RunAsNetOnly == crb_On) cchMaxLen++; // -new_console:e
	cchMaxLen += (pszUserName ? (lstrlen(pszUserName) + 32 // "-new_console:u:<user>:<pwd>"
						+ (pszDomain ? lstrlen(pszDomain) : 0)
						+ wcslen(szUserPassword)) : 0);
	if (ForceUserDialog == crb_On) cchMaxLen++; // -new_console:u
	if (BackgroundTab == crb_On) cchMaxLen++; // -new_console:b
	if (ForegroundTab == crb_On) cchMaxLen++; // -new_console:f
	if (BufHeight == crb_On) cchMaxLen += 32; // -new_console:h<lines>
	if (LongOutputDisable == crb_On) cchMaxLen++; // -new_console:o
	if (OverwriteMode != crb_Off) cchMaxLen += 2; // -new_console:w[0|1]
	cchMaxLen += (nPTY ? 15 : 0); // -new_console:p5
	if (InjectsDisable == crb_On) cchMaxLen++; // -new_console:i
	if (ForceNewWindow == crb_On) cchMaxLen++; // -new_console:N
	if (ForceHooksServer == crb_On) cchMaxLen++; // -new_console:R
	if (eConfirmation) cchMaxLen+=2; // -new_console:c[0|1] / -new_console:n
	if (ForceDosBox == crb_On) cchMaxLen++; // -new_console:x
	if (ForceInherit == crb_On) cchMaxLen++; // -new_console:I
	if (eSplit) cchMaxLen += 64; // -new_console:s[<SplitTab>T][<Percents>](H|V)

	auto* pszFull = result.GetBuffer(cchMaxLen);
	if (!pszFull)
	{
		_ASSERTE(pszFull!=nullptr);
		return {};
	}

	if (pszSpecialCmd && (RunAsAdministrator == crb_On) && abForTasks)
		_wcscpy_c(pszFull, cchMaxLen, L"* "); // `-new_console` will follow asterisk, so add a space to delimit
	else
		*pszFull = 0;


	wchar_t szAdd[128] = L"";
	if (RunAsAdministrator == crb_On)
	{
		// Don't add -new_console:a if the asterisk was already set
		if (*pszFull != L'*')
			wcscat_c(szAdd, L"a");
	}
	else if (RunAsRestricted == crb_On)
	{
		wcscat_c(szAdd, L"r");
	}
	// Used *together* with RunAsAdministrator
	if (RunAsSystem == crb_On)
	{
		wcscat_c(szAdd, L"A");
	}

	if (RunAsNetOnly == crb_On)
	{
		wcscat_c(szAdd, L"e");
	}

	if ((ForceUserDialog == crb_On) && !(pszUserName && *pszUserName))
		wcscat_c(szAdd, L"u");

	if (BackgroundTab == crb_On)
		wcscat_c(szAdd, L"b");
	else if (ForegroundTab == crb_On)
		wcscat_c(szAdd, L"f");

	if (ForceDosBox == crb_On)
		wcscat_c(szAdd, L"x");

	if (ForceInherit == crb_On)
		wcscat_c(szAdd, L"I");

	switch (eConfirmation)
	{
	case eConfAlways:
		wcscat_c(szAdd, L"c"); break;
	case eConfEmpty:
		wcscat_c(szAdd, L"c0"); break;
	case eConfHalt:
		wcscat_c(szAdd, L"c1"); break;
	case eConfNever:
		wcscat_c(szAdd, L"n"); break;
	case eConfDefault:
		break; // Don't add anything
	}

	if (LongOutputDisable == crb_On)
		wcscat_c(szAdd, L"o");

	if (OverwriteMode == crb_On)
		wcscat_c(szAdd, L"w");
	else if (OverwriteMode == crb_Off)
		wcscat_c(szAdd, L"w0");

	if (nPTY == pty_Default)
		wcscat_c(szAdd, L"p");
	else if (nPTY)
		msprintf(szAdd+lstrlen(szAdd), 15, L"p%u", nPTY);

	if (InjectsDisable == crb_On)
		wcscat_c(szAdd, L"i");

	if (ForceNewWindow == crb_On)
		wcscat_c(szAdd, L"N");

	if (ForceHooksServer == crb_On)
		wcscat_c(szAdd, L"R");

	if (BufHeight == crb_On)
	{
		if (nBufHeight)
			msprintf(szAdd+lstrlen(szAdd), 16, L"h%u", nBufHeight);
		else
			wcscat_c(szAdd, L"h");
	}

	// -new_console:s[<SplitTab>T][<Percents>](H|V)
	if (eSplit)
	{
		wcscat_c(szAdd, L"s");
		if (nSplitPane)
			msprintf(szAdd+lstrlen(szAdd), 16, L"%uT", nSplitPane);
		if (nSplitValue > 0 && nSplitValue < 1000)
		{
			const UINT iPercent = (1000 - nSplitValue) / 10;
			msprintf(szAdd+lstrlen(szAdd), 16, L"%u", std::max<UINT>(1, std::min<UINT>(iPercent, 99)));
		}
		wcscat_c(szAdd, (eSplit == eSplitHorz) ? L"H" : L"V");
	}

	// The command itself will be appended at the end
	//   to minimize modification of command line, also we skip all switches after certain executables
	//   for example, only first must be processed (just an example): -cur_console:d:C:\Temp cmd.exe /k ConEmuC /e -cur_console
	// so we add a space AFTER but not before

	if (szAdd[0])
	{
		_wcscat_c(pszFull, cchMaxLen, (NewConsole == crb_On) ? L"-new_console:" : L"-cur_console:");
		_wcscat_c(pszFull, cchMaxLen, szAdd);
		_wcscat_c(pszFull, cchMaxLen, L" ");
	}

	struct CopyValues { wchar_t cOpt; bool bEscape; LPCWSTR pVal; } values[] =
	{
		{L'd', false, this->pszStartupDir},
		{L't', true,  this->pszRenameTab},
		{L'C', false, this->pszIconFile},
		{L'P', true,  this->pszPalette},
		{L'W', false, this->pszWallpaper},
		{L'm', false, this->pszMntRoot},
		{L'L', false, this->pszAnsiLog},
		{}
	};

	wchar_t szCat[32];
	for (CopyValues* p = values; p->cOpt; p++)
	{
		if (p->pVal)
		{
			const bool bQuot = !*p->pVal || (wcspbrk(p->pVal, L" \"") != nullptr);

			if (bQuot)
				msprintf(szCat, countof(szCat), (NewConsole == crb_On) ? L"-new_console:%c:\"" : L"-cur_console:%c:\"", p->cOpt);
			else
				msprintf(szCat, countof(szCat), (NewConsole == crb_On) ? L"-new_console:%c:" : L"-cur_console:%c:", p->cOpt);

			_wcscat_c(pszFull, cchMaxLen, szCat);

			if (p->bEscape)
			{
				wchar_t* pD = pszFull + lstrlen(pszFull);
				const wchar_t* pS = p->pVal;
				while (*pS)
				{
					if (wcschr(CmdEscapeNeededChars/* L"<>()&|^\"" */, *pS))
						*(pD++) = (*pS == L'"') ? L'"' : L'^';
					*(pD++) = *(pS++);
				}
				_ASSERTE(pD < (pszFull+cchMaxLen));
				*pD = 0;
			}
			else
			{
				_wcscat_c(pszFull, cchMaxLen, p->pVal);
			}

			_wcscat_c(pszFull, cchMaxLen, bQuot ? L"\" " : L" ");
		}
	}

	// "-new_console:u:<user>:<pwd>"
	if (pszUserName && *pszUserName)
	{
		_wcscat_c(pszFull, cchMaxLen, (NewConsole == crb_On) ? L"-new_console:u:\"" : L"-cur_console:u:\"");
		if (pszDomain && *pszDomain)
		{
			_wcscat_c(pszFull, cchMaxLen, pszDomain);
			_wcscat_c(pszFull, cchMaxLen, L"\\");
		}
		_wcscat_c(pszFull, cchMaxLen, pszUserName);
		if (*szUserPassword || (ForceUserDialog != crb_On))
		{
			_wcscat_c(pszFull, cchMaxLen, L":");
		}
		if (*szUserPassword)
		{
			_wcscat_c(pszFull, cchMaxLen, szUserPassword);
		}
		_wcscat_c(pszFull, cchMaxLen, L"\" ");
	}

	// See the note above, why we add the command after "-new_console" switches
	// User may modify the command appropriately afterwards
	if (pszSpecialCmd)
	{
		// Don't quotate, the command is up to user
		_wcscat_c(pszFull, cchMaxLen, pszSpecialCmd);
	}

	// Trim trailing spaces
	wchar_t* pS = pszFull + lstrlen(pszFull);
	while ((pS > pszFull) && wcschr(L" \t\r\n", *(pS - 1)))
		*(--pS) = 0;

	return result;
}


bool RConStartArgsEx::CheckUserToken(HWND hPwd)
{
	//SafeFree(pszUserProfile);
	UseEmptyPassword = crb_Undefined;

	//if (hLogonToken) { CloseHandle(hLogonToken); hLogonToken = nullptr; }
	if (!pszUserName || !*pszUserName)
		return FALSE;

	//wchar_t szPwd[MAX_PATH]; szPwd[0] = 0;
	//szUserPassword[0] = 0;

	if (!GetWindowText(hPwd, szUserPassword, MAX_PATH-1))
	{
		szUserPassword[0] = 0;
		UseEmptyPassword = crb_On;
	}
	else
	{
		UseEmptyPassword = crb_Off;
	}

	SafeFree(pszDomain);
	wchar_t* pszSlash = wcschr(pszUserName, L'\\');
	if (pszSlash)
	{
		pszDomain = pszUserName;
		*pszSlash = 0;
		pszUserName = lstrdup(pszSlash + 1).Detach();
	}

	HANDLE hLogonToken = CheckUserToken();
	const bool bIsValid = (hLogonToken != nullptr);
	// Token itself is not needed now
	SafeCloseHandle(hLogonToken);

	return bIsValid;
}


HANDLE RConStartArgsEx::CheckUserToken()
{
	HANDLE hLogonToken = nullptr;
	// aka: code 1327 (ERROR_ACCOUNT_RESTRICTION)
	// If user needs to use empty password (THINK TWICE! It's a security hole!)
	// gpedit.msc - Конфигурация компьютера - Конфигурация Windows - Локальные политики - Параметры безопасности - Учетные записи
	// Ограничить использование пустых паролей только для консольного входа -> "Отключить".
	const auto* pszPassword = (UseEmptyPassword == crb_On) ? nullptr : szUserPassword;
	const DWORD nFlags = (RunAsNetOnly == crb_On) ? LOGON32_LOGON_NEW_CREDENTIALS : LOGON32_LOGON_INTERACTIVE;
	const BOOL lbRc = LogonUser(pszUserName, pszDomain, pszPassword, nFlags, LOGON32_PROVIDER_DEFAULT, &hLogonToken);

	if (!lbRc || !hLogonToken)
	{
		return nullptr;
	}

	return hLogonToken;
}
