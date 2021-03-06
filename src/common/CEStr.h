
/*
Copyright (c) 2013-present Maximus5
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

#include "defines.h"
#include "MArray.h"

// CEStr
struct CEStr
{
// #CEStr Make ms_Val protected
public:
	wchar_t *ms_Val = nullptr;
private:
	ssize_t maxCount_ = 0; // Including termination \0

private:
	const wchar_t* AttachInt(wchar_t*& asPtr);

public:
	CEStr();
	CEStr(CEStr&& asStr) noexcept;
	CEStr(const CEStr& asStr);
	CEStr(const wchar_t* asStr1, const wchar_t* asStr2 = nullptr, const wchar_t* asStr3 = nullptr,
		const wchar_t* asStr4 = nullptr, const wchar_t* asStr5 = nullptr, const wchar_t* asStr6 = nullptr,
		const wchar_t* asStr7 = nullptr, const wchar_t* asStr8 = nullptr, const wchar_t* asStr9 = nullptr);
	CEStr(const wchar_t* asStr, ssize_t anChars);

	CEStr& operator=(CEStr&& asStr) noexcept;
	CEStr& operator=(const CEStr& asStr);
	CEStr& operator=(const wchar_t* asPtr);

	~CEStr();

	bool IsEmpty() const;
	bool IsNull() const;

	operator const wchar_t*() const;
	operator bool() const;
	// ReSharper disable once CppInconsistentNaming
	const wchar_t* c_str(const wchar_t* asNullSubstitute = nullptr) const;
	// ReSharper disable once CppInconsistentNaming
	wchar_t* data() const;
	const wchar_t* Right(ssize_t cchMaxCount) const;
	const wchar_t* Mid(ssize_t cchOffset) const;

	int Compare(const wchar_t* asText, bool abCaseSensitive = false) const;
	bool operator==(const wchar_t* asStr) const;

	void swap(CEStr& asStr) noexcept;

	ssize_t GetLen() const;
	ssize_t GetMaxCount();

	/// <summary>
	/// Returns a buffer capable to allocate <ref>cchMaxLen</ref> chars + zero-terminator.
	/// </summary>
	/// <param name="cchMaxLen">Required characters count without zero-terminator.</param>
	/// <returns>Allocated buffer or nullptr</returns>
	wchar_t* GetBuffer(ssize_t cchMaxLen);
	wchar_t* Detach();
	void Swap(wchar_t*& asPtr);
	const wchar_t*  Attach(wchar_t*&& asPtr);
	const wchar_t*  Append(const wchar_t* asStr1, const wchar_t* asStr2 = nullptr, const wchar_t* asStr3 = nullptr,
		const wchar_t* asStr4 = nullptr, const wchar_t* asStr5 = nullptr, const wchar_t* asStr6 = nullptr,
		const wchar_t* asStr7 = nullptr, const wchar_t* asStr8 = nullptr);
	CEStr& Replace(const wchar_t* what, const wchar_t* newText);
	/// @brief If string allocated, make it zero-length.
	void Clear();
	/// @brief Release allocated memory, so the object contains nullptr afterwards.
	void Release();
	/// @brief Assigns asNewValue, possibly limited to anChars
	const wchar_t* Set(const wchar_t* asNewValue, ssize_t anChars = -1);
	/// @brief Sets the character wc at position nIdx
	/// @returns Previous character at position nIdx
	wchar_t SetAt(const ssize_t nIdx, const wchar_t wc);
};


/// <summary>
/// Minimalistic storage for ANSI/UTF8 strings
/// </summary>
struct CEStrA
{
public:
	CEStrA();
	~CEStrA();
	CEStrA(const char* asPtr);
	CEStrA(const char* asStr1, const char* asStr2, const char* asStr3 = nullptr,
		const char* asStr4 = nullptr, const char* asStr5 = nullptr, const char* asStr6 = nullptr,
		const char* asStr7 = nullptr, const char* asStr8 = nullptr, const char* asStr9 = nullptr);

	CEStrA(const CEStrA& src);
	CEStrA(CEStrA&& src) noexcept;

	CEStrA& operator=(const char* asPtr);

	CEStrA& operator=(const CEStrA& src);
	CEStrA& operator=(CEStrA&& src) noexcept;

	operator const char*() const;
	operator bool() const;
	// ReSharper disable once CppInconsistentNaming
	const char* c_str(const char* asNullSubstitute = nullptr) const;
	// ReSharper disable once CppInconsistentNaming
	char* data() const;
	ssize_t GetLen() const;
	ssize_t GetMaxCount();
	void Clear();
	void Release();

	const char* Set(const char* asNewValue, ssize_t anChars = -1);
	char SetAt(const ssize_t nIdx, const char chr);

	// Reset the buffer to new empty data of required size
	char* GetBuffer(ssize_t cchMaxLen);
	char* Detach();
	char* Attach(char*&& asPtr);

public:
	char* ms_Val = nullptr;
private:
	ssize_t maxCount_ = 0; // Including termination \0
};


/// <summary>
/// Implements string concatenation
/// </summary>
// ReSharper disable once CppInconsistentNaming
class CEStrConcat
{
private:
	MArray<std::pair<ssize_t, CEStr>> strings_;
	ssize_t total_ = 0;
public:
	void Reserve(ssize_t strCount);
	void Append(const wchar_t* str);
	void Append(CEStr&& str);
	CEStr GetData() const;
	bool IsEmpty() const;
	ssize_t GetCount() const;
	CEStr& GetString(ssize_t idx);
};
