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

#include "defines.h"
#include "MAssert.h"
#include "MStrSafe.h"
#include "CEStr.h"

// Используется в ConEmuC.exe, и для минимизации кода
// менеджер памяти тут использоваться не должен!
#undef malloc
#define malloc "malloc"
#undef calloc
#define calloc "calloc"

#ifdef _DEBUG
	#ifndef __GNUC__
		#pragma comment(lib,"strsafe.lib")
	#endif
#endif

// ReSharper disable once CppInconsistentNaming
const wchar_t* msprintf(wchar_t* lpOut, const size_t cchOutMax, const wchar_t* lpFmt, ...)
{
	va_list argptr;
	va_start(argptr, lpFmt);
	const auto* result = mvsprintf(lpOut, cchOutMax, lpFmt, argptr);
	va_end(argptr);
	return result;
}

const wchar_t* mvsprintf(wchar_t* lpOut, size_t cchOutMax, const wchar_t* lpFmt, va_list argptr)
{
	if (!lpOut || !cchOutMax)
		return nullptr;
	
	const wchar_t* pszSrc = lpFmt;
	wchar_t* pszDst = lpOut;
	const wchar_t* endPtr = lpOut + (cchOutMax - 1); // maximum available write position ('\0')
	bool overflow = false;
	wchar_t  szValue[16];
	wchar_t* pszValue;

	while (*pszSrc)
	{
		if (pszDst >= endPtr)
		{
			_ASSERTE(FALSE && "output buffer overflow");
			overflow = true;
			break;
		}
		
		if (*pszSrc == L'%')
		{
			pszSrc++;
			switch (*pszSrc)
			{
			case L'%':
				{
					*(pszDst++) = L'%';
					pszSrc++;
					continue;
				}
			case L'c':
				{
					// GCC: 'wchar_t' is promoted to 'int' when passed through '...'
					const wchar_t c = va_arg( argptr, int );
					*(pszDst++) = c;
					pszSrc++;
					continue;
				}
			case L's':
				{
					pszValue = va_arg( argptr, wchar_t* );
					if (pszValue)
					{
						while (*pszValue)
						{
							if (pszDst >= endPtr)
							{
								_ASSERTE(FALSE && "output buffer overflow");
								overflow = true;
								break;
							}
							*(pszDst++) = *(pszValue++);
						}
					}
					pszSrc++;
					continue;
				}
			case L'S':
				{
					char* pszValueA = va_arg( argptr, char* );
					if (pszValueA)
					{
						while (*pszValueA)
						{
							_ASSERTE(*pszValueA <= 0x7F); // Should be used only for debugging purposes
							if (pszDst >= endPtr)
							{
								_ASSERTE(FALSE && "output buffer overflow");
								overflow = true;
								break;
							}
							*(pszDst++) = static_cast<wchar_t>(*(pszValueA++));
						}
					}
					pszSrc++;
					continue;
				}
			case L'u':
			case L'i':
				{
					UINT nValue = 0;
					if (*pszSrc == L'i')
					{
						const int n = va_arg( argptr, int );
						if (n < 0)
						{
							*(pszDst++) = L'-';
							nValue = -n;
						}
						else
						{
							nValue = n;
						}
					}
					else
					{
						nValue = va_arg( argptr, UINT );
					}
					pszSrc++;
					pszValue = szValue;
					while (nValue)
					{
						const WORD n = static_cast<WORD>(nValue % 10);
						*(pszValue++) = static_cast<wchar_t>(L'0' + n);
						nValue = (nValue - n) / 10;
					}
					if (pszValue == szValue)
						*(pszValue++) = L'0';
					// Move data to output buffer
					while (pszValue > szValue)
					{
						if (pszDst >= endPtr)
						{
							_ASSERTE(FALSE && "output buffer overflow");
							overflow = true;
							break;
						}
						*(pszDst++) = *(--pszValue);
					}
					continue;
				}
			case L'0':
			case L'X':
			case L'x':
				{
					int nLen = 0;
					// ReSharper disable once CppJoinDeclarationAndAssignment
					DWORD nValue;
					wchar_t cBase = L'A';
					if (pszSrc[0] == L'0' && isDigit(pszSrc[1]) && (pszSrc[2] == L'X' || pszSrc[2] == L'x'))
					{
						if (pszSrc[2] == L'x')
							cBase = L'a';
						if (pszSrc[1] == L'8')
						{
							memmove(szValue, L"00000000", 16);
							nLen = 8;
						}
						else if (pszSrc[1] == L'4')
						{
							memmove(szValue, L"0000", 8);
							nLen = 4;
						}
						else if (pszSrc[1] == L'2')
						{
							memmove(szValue, L"00", 4);
							nLen = 2;
						}
						else
						{
							_ASSERTE(FALSE && "Unsupported %0?X format");
						}
						pszSrc += 3;
					}
					else if (pszSrc[0] == L'0' && (pszSrc[1] == L'2' || pszSrc[1] == L'3') && pszSrc[2] == L'u')
					{
						cBase = 0;
						szValue[0] = 0;
						nLen = static_cast<int>(pszSrc[1]) - static_cast<int>(L'0');
						pszSrc += 3;
					}
					else if (pszSrc[0] == L'X' || pszSrc[0] == L'x')
					{
						if (pszSrc[0] == L'x')
							cBase = L'a';
						pszSrc++;
					}
					else
					{
						_ASSERTE(*pszSrc == L'u' || *pszSrc == L's' || *pszSrc == L'c' || *pszSrc == L'i' || *pszSrc == L'X');
						goto wrap;
					}
					
					
					nValue = va_arg( argptr, UINT );
					pszValue = szValue;
					if (cBase)
					{
						// Hexadecimal branch
						while (nValue)
						{
							const WORD n = static_cast<WORD>(nValue & 0xF);
							if (n <= 9)
								*(pszValue++) = static_cast<wchar_t>(L'0' + n);
							else
								*(pszValue++) = static_cast<wchar_t>(cBase + n - 10);
							nValue = nValue >> 4;
						}
						int nCurLen = static_cast<int>(pszValue - szValue);
						if (!nLen)
						{
							nLen = nCurLen;
							if (!nLen)
							{
								*(pszValue++) = L'0';
								nLen = 1;
							}
						}
						else
						{
							pszValue = (szValue+std::max(nLen,nCurLen));
						}
						// move result to Dest
						while (pszValue > szValue)
						{
							if (pszDst >= endPtr)
							{
								_ASSERTE(FALSE && "output buffer overflow");
								overflow = true;
								break;
							}
							*(pszDst++) = *(--pszValue);
						}
					}
					else
					{
						// Decimal branch
						int nGetLen = 0;
						while (nValue)
						{
							const WORD n = static_cast<WORD>(nValue % 10);
							*(pszValue++) = static_cast<wchar_t>(L'0' + n);
							nValue = (nValue - n) / 10;
							nGetLen++;
						}
						while ((nGetLen++) < nLen)
						{
							if (pszDst >= endPtr)
							{
								_ASSERTE(FALSE && "output buffer overflow");
								overflow = true;
								break;
							}
							*(pszDst++) = L'0';
						}
						while ((--pszValue) >= szValue)
						{
							if (pszDst >= endPtr)
							{
								_ASSERTE(FALSE && "output buffer overflow");
								overflow = true;
								break;
							}
							*(pszDst++) = *pszValue;
						}
					}
					continue;
				}
			default:
				_ASSERTE(*pszSrc == L'u' || *pszSrc == L's' || *pszSrc == L'c' || *pszSrc == L'i' || *pszSrc == L'X');
			}
		}
		else
		{
			*(pszDst++) = *(pszSrc++);
		}
	}
wrap:
	_ASSERTE(pszDst <= endPtr);
	*pszDst = 0;
	_ASSERTE(lstrlen(lpOut) < static_cast<int>(cchOutMax));
	return overflow ? nullptr : lpOut;
}

const char* msprintf(char* lpOut, const size_t cchOutMax, const char* lpFmt, ...)
{
	va_list argptr;
	va_start(argptr, lpFmt);
	const auto* result = mvsprintf(lpOut, cchOutMax, lpFmt, argptr);
	va_end(argptr);
	return result;
}

const char* mvsprintf(char* lpOut, size_t cchOutMax, const char* lpFmt, va_list argptr)
{
	if (!lpOut || !cchOutMax)
		return nullptr;
	
	const char* pszSrc = lpFmt;
	char* pszDst = lpOut;
	const char* endPtr = lpOut + (cchOutMax - 1); // maximum available write position ('\0')
	bool overflow = false;
	char  szValue[16];
	char* pszValue;

	while (*pszSrc)
	{
		if (pszDst >= endPtr)
		{
			_ASSERTE(FALSE && "output buffer overflow");
			overflow = true;
			break;
		}
		
		if (*pszSrc == '%')
		{
			pszSrc++;
			switch (*pszSrc)
			{
			case '%':
				{
					*(pszDst++) = '%';
					pszSrc++;
					continue;
				}
			case 'c':
				{
					// GCC: 'wchar_t' is promoted to 'int' when passed through '...'
					const char c = va_arg( argptr, int );
					*(pszDst++) = c;
					pszSrc++;
					continue;
				}
			case 's':
				{
					pszValue = va_arg( argptr, char* );
					if (pszValue)
					{
						while (*pszValue)
						{
							if (pszDst >= endPtr)
							{
								_ASSERTE(FALSE && "output buffer overflow");
								overflow = true;
								break;
							}
							*(pszDst++) = *(pszValue++);
						}
					}
					pszSrc++;
					continue;
				}
			case 'S':
				{
					wchar_t* pszValueW = va_arg( argptr, wchar_t* );
					if (pszValueW)
					{
						while (*pszValueW)
						{
							_ASSERTE(*pszValueW <= 0x7F); // Should be used only for debugging purposes
							if (pszDst >= endPtr)
							{
								_ASSERTE(FALSE && "output buffer overflow");
								overflow = true;
								break;
							}
							*(pszDst++) = static_cast<char>(*(pszValueW++));
						}
					}
					pszSrc++;
					continue;
				}
			case 'u':
			case 'i':
				{
					UINT nValue = 0;
					if (*pszSrc == 'i')
					{
						const int n = va_arg( argptr, int );
						if (n < 0)
						{
							*(pszDst++) = '-';
							nValue = -n;
						}
						else
						{
							nValue = n;
						}
					}
					else
					{
						nValue = va_arg( argptr, UINT );
					}
					pszSrc++;
					pszValue = szValue;
					while (nValue)
					{
						const WORD n = static_cast<WORD>(nValue % 10);
						*(pszValue++) = static_cast<char>('0' + n);
						nValue = (nValue - n) / 10;
					}
					if (pszValue == szValue)
						*(pszValue++) = '0';
					// Move data to result buffer
					while (pszValue > szValue)
					{
						if (pszDst >= endPtr)
						{
							_ASSERTE(FALSE && "output buffer overflow");
							overflow = true;
							break;
						}
						*(pszDst++) = *(--pszValue);
					}
					continue;
				}
			case '0':
			case 'X':
			case 'x':
				{
					int nLen = 0;
					// ReSharper disable once CppJoinDeclarationAndAssignment
					DWORD nValue;
					char cBase = 'A';
					if (pszSrc[0] == '0' && isDigit(pszSrc[1]) && (pszSrc[2] == 'X' || pszSrc[2] == 'x'))
					{
						if (pszSrc[2] == 'x')
							cBase = 'a';
						if (pszSrc[1] == '8')
						{
							memmove(szValue, "00000000", 8);
							nLen = 8;
						}
						else if (pszSrc[1] == '4')
						{
							memmove(szValue, "0000", 4);
							nLen = 4;
						}
						else if (pszSrc[1] == '2')
						{
							memmove(szValue, "00", 2);
							nLen = 2;
						}
						else
						{
							_ASSERTE(FALSE && "Unsupported %0?X format");
						}
						pszSrc += 3;
					}
					else if (pszSrc[0] == '0' && (pszSrc[1] == '2' || pszSrc[1] == '3') && pszSrc[2] == 'u')
					{
						cBase = 0;
						szValue[0] = 0;
						nLen = static_cast<int>(pszSrc[1]) - static_cast<int>('0');
						pszSrc += 3;
					}
					else if (pszSrc[0] == 'X' || pszSrc[0] == 'x')
					{
						if (pszSrc[0] == 'x')
							cBase = 'a';
						pszSrc++;
					}
					else
					{
						_ASSERTE(FALSE && "Unsupported modifier");
						goto wrap;
					}
					
					
					nValue = va_arg( argptr, UINT );
					pszValue = szValue;
					if (cBase)
					{
						// Hexadecimal branch
						while (nValue)
						{
							const WORD n = static_cast<WORD>(nValue & 0xF);
							if (n <= 9)
								*(pszValue++) = static_cast<char>('0' + n);
							else
								*(pszValue++) = static_cast<char>(cBase + n - 10);
							nValue = nValue >> 4;
						}
						int nCurLen = static_cast<int>(pszValue - szValue);
						if (!nLen)
						{
							nLen = nCurLen;
							if (!nLen)
							{
								*(pszValue++) = '0';
								nLen = 1;
							}
						}
						else
						{
							pszValue = (szValue+std::max(nLen,nCurLen));
						}
						// move result to Dest
						while (pszValue > szValue)
						{
							if (pszDst >= endPtr)
							{
								_ASSERTE(FALSE && "output buffer overflow");
								overflow = true;
								break;
							}
							*(pszDst++) = *(--pszValue);
						}
					}
					else
					{
						// Decimal branch
						int nGetLen = 0;
						while (nValue)
						{
							const WORD n = static_cast<WORD>(nValue % 10);
							*(pszValue++) = static_cast<char>('0' + n);
							nValue = (nValue - n) / 10;
							nGetLen++;
						}
						while ((nGetLen++) < nLen)
						{
							if (pszDst >= endPtr)
							{
								_ASSERTE(FALSE && "output buffer overflow");
								overflow = true;
								break;
							}
							*(pszDst++) = '0';
						}
						while ((--pszValue) >= szValue)
						{
							if (pszDst >= endPtr)
							{
								_ASSERTE(FALSE && "output buffer overflow");
								overflow = true;
								break;
							}
							*(pszDst++) = *pszValue;
						}
					}
					continue;
				}
			default:
				_ASSERTE(*pszSrc == 'u' || *pszSrc == 's' || *pszSrc == 'c' || *pszSrc == 'i' || *pszSrc == 'X');
			}
		}
		else
		{
			*(pszDst++) = *(pszSrc++);
		}
	}
wrap:
	_ASSERTE(pszDst <= endPtr);
	*pszDst = 0;
	_ASSERTE(lstrlenA(lpOut) < static_cast<int>(cchOutMax));
	return overflow ? nullptr : lpOut;
}

static int CompareStringPointers(const void* asStr1, const void* asStr2)
{
	if (asStr1 == nullptr && asStr2)
		return -1; // str1 is lesser than str2
	if (asStr1 && asStr2 == nullptr)
		return 1; // str1 is greater than str2
	return 0;
}

// ReSharper disable once CppInconsistentNaming
int lstrcmpni(LPCSTR asStr1, LPCSTR asStr2, int cchMax)
{
	if (!asStr1 || !asStr2 || cchMax <= 0)
	{
		return CompareStringPointers(asStr1, asStr2);
	}

	const int stackMaxLen = 64;
	char sz1[stackMaxLen], sz2[stackMaxLen];
	const char* ptr1; const char* ptr2;
	CEStrA buf1, buf2;

	int nCmp = 0;

	if (cchMax >= stackMaxLen)
	{
		ptr1 = buf1.Set(asStr1, cchMax);
		ptr2 = buf2.Set(asStr2, cchMax);
	}
	else
	{
		ptr1 = lstrcpynA(sz1, asStr1, cchMax + 1);
		ptr2 = lstrcpynA(sz2, asStr2, cchMax + 1);
	}

	if (!ptr1 || !ptr2)
	{
		_ASSERTE(ptr1 && ptr2);
		nCmp = CompareStringPointers(ptr1, ptr2);
	}
	else
	{
		nCmp = lstrcmpiA(ptr1, ptr2);
	}

	return nCmp;
}

int lstrcmpni(LPCWSTR asStr1, LPCWSTR asStr2, int cchMax)
{
	if (!asStr1 || !asStr2 || cchMax <= 0)
	{
		return CompareStringPointers(asStr1, asStr2);
	}

	const int stackMaxLen = 64;
	wchar_t sz1[stackMaxLen], sz2[stackMaxLen];
	const wchar_t* ptr1; const wchar_t* ptr2;
	CEStr buf1, buf2;

	int nCmp = 0;

	if (cchMax >= stackMaxLen)
	{
		ptr1 = buf1.Set(asStr1, cchMax);
		ptr2 = buf2.Set(asStr2, cchMax);
	}
	else
	{
		ptr1 = lstrcpynW(sz1, asStr1, cchMax + 1);
		ptr2 = lstrcpynW(sz2, asStr2, cchMax + 1);
	}

	if (!ptr1 || !ptr2)
	{
		_ASSERTE(ptr1 && ptr2);
		nCmp = CompareStringPointers(ptr1, ptr2);
	}
	else
	{
		nCmp = lstrcmpi(ptr1, ptr2);
	}

	return nCmp;
}

// Returns 0 - if does not match
// If match - returns length of the asPattern
int startswith(LPCWSTR asStr, LPCWSTR asPattern, bool abIgnoreCase)
{
	if (!asStr || !*asStr || !asPattern || !*asPattern)
		return 0;
	int iCmp;
	const int iLen = lstrlen(asPattern);
	if (abIgnoreCase)
		iCmp = lstrcmpni(asStr, asPattern, iLen);
	else
		iCmp = wcsncmp(asStr, asPattern, iLen);
	return (iCmp == 0) ? iLen : 0;
}

//#if defined(_DEBUG) && !defined(STRSAFE_DISABLE)
int swprintf_c(wchar_t* Buffer, INT_PTR size, const wchar_t *Format, ...)
{
	_ASSERTE(Buffer!=Format);
	if (size < 0)
		DebugBreak();
	va_list argList;
	va_start(argList, Format);
	const int nRc = StringCchVPrintfW(Buffer, size, Format, argList);
	va_end(argList);
	return nRc;
}

int sprintf_c(char* Buffer, INT_PTR size, const char *Format, ...)
{
	_ASSERTE(Buffer!=Format);
	if (size < 0)
		DebugBreak();
	va_list argList;
	va_start(argList, Format);
	const int nRc = StringCchVPrintfA(Buffer, size, Format, argList);
	va_end(argList);
	return nRc;
}
//#endif // #if defined(_DEBUG) && !defined(STRSAFE_DISABLE)

#if 0 //defined(__CYGWIN__)

wchar_t* _itow(int value, wchar_t *str, int radix)
{
	if (radix == 16)
	{
		wsprintf(str, L"%X", value);
	}
	else if (radix == 10)
	{
		wsprintf(str, L"%i", value);
	}
	else
	{
		_ASSERTE(FALSE && "Not supported in CygWin");
		*str = 0;
	}
	return str;
}

int _wtoi(const wchar_t *str)
{
	//char sz[32] = "";
	//WideCharToMultiByte(CP_ACP, 0, str, -1, sz, 32, NULL, NULL);
	//return _atoi(sz);

	//int i = 0;
	//swscanf(str, "%d", &i);
	//return i;

	wchar_t* p;
	return wcstol(str, &p, 10);
}
#endif
