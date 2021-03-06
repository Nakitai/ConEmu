
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


#define HIDE_USE_EXCEPTION_INFO
#define SHOWDEBUGSTR

#include "Header.h"

#include "ColorFix.h"
#include "Options.h"
#include "RConPalette.h"
#include "RealConsole.h"
#include "SetDlgColors.h"

using ConEmu::PaletteColors;

CRConPalette::CRConPalette(CRealConsole* apRCon)
	: mp_RCon(apRCon)
{
}

CRConPalette::~CRConPalette()  // NOLINT(modernize-use-equals-default)
{
}

void CRConPalette::UpdateColorTable(const PaletteColors& colors,
		bool bVividColors,
		bool bExtendFonts, BYTE nFontNormalColor, BYTE nFontBoldColor, BYTE nFontItalicColor)
{
	bool bMainChanged = !mb_Initialized
		|| (mb_VividColors != bVividColors)
		|| (m_Colors != colors);
	bool bExtChanged = bMainChanged
		|| ((mb_ExtendFonts != bExtendFonts)
			|| (bExtendFonts
				&& ((mn_FontNormalColor != nFontNormalColor)
					|| (mn_FontBoldColor != nFontBoldColor)
					|| (mn_FontItalicColor != nFontItalicColor))))
		;

	if (!bExtChanged)
		return; // Nothing to update

	// The actions
	CharAttr lca;

	if (bMainChanged)
	{
		real_type oldDE = 0, newDE = 0;
		wchar_t szLog[200];
		uint32_t nColorIndex = 0;

		for (uint32_t nBack = 0; nBack <= 0xF; nBack++)
		{
			for (uint32_t nFore = 0; nFore <= 0xF; nFore++, nColorIndex++)
			{
				lca = CharAttr{};
				lca.nForeIdx = nFore;
				lca.nBackIdx = nBack;
				lca.crForeColor = lca.crOrigForeColor = colors[lca.nForeIdx];
				lca.crBackColor = lca.crOrigBackColor = colors[lca.nBackIdx];

				// New feature, apply modified lightness for text colors
				// if text color is indistinguishable with background
				if (bVividColors && (lca.crForeColor != lca.crBackColor))
				{
					ColorFix textColor(lca.crForeColor);
					ColorFix vivid;
					if (textColor.PerceivableColor(lca.crBackColor, vivid, &oldDE, &newDE))
					{
						if (mp_RCon->isLogging())
						{
							swprintf_s(szLog, L"VividColor [Bg=%u] {%u %u %u} [Fg=%u] {%u %u %u} [DE=%.2f] >> {%u %u %u} [DE=%.2f]",
								nBack, getR(lca.crBackColor), getG(lca.crBackColor), getB(lca.crBackColor),
								nFore, textColor.r, textColor.g, textColor.b, oldDE,
								vivid.r, vivid.g, vivid.b, newDE);
							mp_RCon->LogString(szLog);
						}

						lca.crForeColor = vivid.rgb;
					}
				}

				m_TableOrg[nColorIndex] = lca;
			}
		}
	}

	memmove_s(m_TableExt, sizeof(m_TableExt), m_TableOrg, sizeof(m_TableExt));

	// And ExtendFonts/ExtendColors features (if required)
	// m_TableExt is the same as m_TableOrg with exception of nFontBoldColor and nFontItalicColor backrounds
	if (bExtendFonts)
	{
		// We need to update only tuples for nFontBoldColor and nFontItalicColor
		for (uint32_t i = 0; i <= 1; i++)
		{
			uint32_t nBack = (i == 0) ? nFontBoldColor : nFontItalicColor;
			if (nBack > 15)
			{
				// ‘None’ was selected
				_ASSERTE(nBack == 0xFF);
				continue;
			}
			CEFontStyles font = (i == 0) ? fnt_Bold : fnt_Italic;
			uint32_t nColorIndex = nBack * 0x10;

			for (uint32_t nFore = 0; nFore <= 0xF; nFore++, nColorIndex++)
			{
				if (nFontNormalColor <= 0x0F)
				{
					// Change background to selected color
					lca = m_TableOrg[nFontNormalColor * 0x10 + nFore];
				}
				else
				{
					// Don't change backround
					_ASSERTE(nFontNormalColor == 0xFF);
					lca = m_TableOrg[nColorIndex];
				}

				// Apply font style (bold/italic)
				lca.nFontIndex = font;

				// -- store ‘real’ color palette indexes?
				//lca.nForeIdx = nFore;
				//lca.nBackIdx = nBack;
				//lca.crOrigForeColor = colors[lca.nForeIdx];
				//lca.crOrigBackColor = colors[lca.nBackIdx];

				m_TableExt[nColorIndex] = lca;
			}
		}
	} // if (bExtChanged)

	// Done, remember settings
	mb_Initialized = true;
	mb_VividColors = bVividColors;
	mb_ExtendFonts = bExtendFonts;
	mn_FontNormalColor = nFontNormalColor;
	mn_FontBoldColor = nFontBoldColor;
	mn_FontItalicColor = nFontItalicColor;
	m_Colors = colors;
}
