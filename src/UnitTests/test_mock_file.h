
/*
Copyright (c) 2020-present Maximus5
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

#include <unordered_map>
#include <unordered_set>
#include <string>

namespace test_mocks
{
	class FileSystemMock final
	{
	public:
		FileSystemMock();
		~FileSystemMock();

		FileSystemMock(const FileSystemMock&) = delete;
		FileSystemMock(FileSystemMock&&) = delete;
		FileSystemMock& operator=(const FileSystemMock&) = delete;
		FileSystemMock& operator=(FileSystemMock&&) = delete;

		void Reset();

		void MockFile(const std::wstring& filePath, uint64_t size = 512, uint32_t subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI, uint32_t bitness = 32);
		void MockDirectory(const std::wstring& directoryPath);

		// Used to simulate files found by %PATH% search
		void MockPathFile(const std::wstring& fileName, const std::wstring& filePath, uint64_t size = 512, uint32_t subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI, uint32_t bitness = 32);

		struct FileInfo
		{
			uint64_t size = 512;
			uint32_t subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
			uint32_t bitness = 32;
		};

		const FileInfo* HasFilePath(const std::wstring& filePath) const;
		bool HasDirectoryPath(const std::wstring& directoryPath) const;
		std::wstring FindInPath(const std::wstring& fileName, const wchar_t* fileExtension) const;

	protected:
		static std::wstring MakeCanonic(const std::wstring& filePath);
		
		std::unordered_map<std::wstring, FileInfo> files_;
		std::unordered_set<std::wstring> directories_;
		std::unordered_map<std::wstring, std::wstring> fileToPath_;
	};
}
