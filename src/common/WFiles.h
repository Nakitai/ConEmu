
/*
Copyright (c) 2014-present Maximus5
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

#include "CmdLine.h"
#include <functional>

bool FilesExists(LPCWSTR asDirectory, LPCWSTR asFileList, bool abAll = false, int anListCount = -1);
bool FileExistSubDir(LPCWSTR asDirectory, LPCWSTR asFile, int iDepth, CEStr& rsFound);
bool EnumFiles(
	LPCWSTR directory, LPCWSTR fileMask, const std::function<bool(const CEStr& filePath, const WIN32_FIND_DATAW& fnd, void* context)>& callback,
	void* context, int depth = 1);
bool DirectoryExists(LPCWSTR asPath);
bool MyCreateDirectory(wchar_t* asPath);
CEStr GetCurDir();

bool IsDotsName(LPCWSTR asName);

class CEnvRestorer;
bool SearchAppPaths(LPCWSTR asFilePath, CEStr& rsFound, bool abSetPath, CEnvRestorer* rpsPathRestore = nullptr);

CEStr GetFullPathNameEx(LPCWSTR asPath);
bool FindFileName(LPCWSTR asPath, CEStr& rsName);
bool MakePathProperCase(CEStr& rsPath);

int ReadTextFile(LPCWSTR asPath, DWORD cchMax, CEStr& rsBuffer, DWORD& rnChars, DWORD& rnErrCode, DWORD DefaultCP = 0);
int ReadTextFile(LPCWSTR asPath, DWORD cchMax, CEStrA& rsBuffer, DWORD& rnChars, DWORD& rnErrCode);
int WriteTextFile(LPCWSTR asPath, const wchar_t* asBuffer, int anSrcLen = -1, DWORD OutCP = CP_UTF8, bool WriteBOM = true, LPDWORD rnErrCode = nullptr);

bool FileCompare(LPCWSTR asFilePath1, LPCWSTR asFilePath2);

#if 0
int apiCancelIoEx(HANDLE hFile, LPOVERLAPPED lpOverlapped);
#endif
int apiCancelSynchronousIo(HANDLE hThread);

bool HasZoneIdentifier(LPCWSTR asFile, int& nZoneID);
bool DropZoneIdentifier(LPCWSTR asFile, DWORD& nErrCode);
