
/*
Copyright (c) 2016-present Maximus5
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

#include "../common/defines.h"

#include "../common/PaletteColors.h"
#include "../common/RgnDetect.h"

class CRealConsole;

constexpr size_t CE_COLOR_PALETTE_TABLE_SIZE = ConEmu::CE_COLOR_PALETTE_SIZE * ConEmu::CE_COLOR_PALETTE_SIZE; // 256

class CRConPalette
{
public:
	CRealConsole* mp_RCon = nullptr;
	CharAttr m_TableOrg[CE_COLOR_PALETTE_TABLE_SIZE]{};
	CharAttr m_TableExt[CE_COLOR_PALETTE_TABLE_SIZE]{};
	ConEmu::PaletteColors m_Colors{};

protected:
	bool mb_Initialized = false;
	bool mb_VividColors = false;
	bool mb_ExtendFonts = false;
	BYTE mn_FontNormalColor = 0, mn_FontBoldColor = 0, mn_FontItalicColor = 0;

public:
	CRConPalette(CRealConsole* apRCon);
	virtual ~CRConPalette();

public:
	// Methods
	void UpdateColorTable(
		const ConEmu::PaletteColors& colors, bool bVividColors,
		bool bExtendFonts, BYTE nFontNormalColor, BYTE nFontBoldColor, BYTE nFontItalicColor);

};
