/*
Copyright (C) 2011, 2012, 2013 Electronic Arts, Inc.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1.  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
3.  Neither the name of Electronic Arts, Inc. ("EA") nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ELECTRONIC ARTS AND ITS CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include <internal/include/EAWebKitJSTextWrapper.h>
#include <EAWebKit/EAWebKit.h>
#include <EAWebKit/EAWebKitTextInterface.h> 
#include <internal/include/EAWebKitAssert.h>
#include <internal/include/EAWebKit_p.h>

COMPILE_ASSERT(sizeof(JSText::Char)			== sizeof(EA::WebKit::Char),		TextCharMismatch );
COMPILE_ASSERT((int)JSText::kCCOtherSymbol	== (int)EA::WebKit::kCCOtherSymbol, TextCharCategoryEnumMismatch );
COMPILE_ASSERT((int)JSText::kBidiClassPDF	== (int)EA::WebKit::kBidiClassPDF,	TextBidiEnumMismatch );
COMPILE_ASSERT((int)JSText::kCaseTypeLower	== (int)EA::WebKit::kCaseTypeLower,	TextCaseEnumMismatch );
COMPILE_ASSERT((int)JSText::kCaseTypeUpper	== (int)EA::WebKit::kCaseTypeUpper,	TextCaseEnumMismatch );
COMPILE_ASSERT((int)JSText::kCaseTypeTitle	== (int)EA::WebKit::kCaseTypeTitle,	TextCaseEnumMismatch );



namespace EA
{
namespace Internal
{

uint32_t JSTextWrapper::GetCharCategory(char16_t c)
{
    EA::WebKit::ITextSystem* pTS = EA::WebKit::GetTextSystem();
    return pTS->GetCharCategory(c);
}

uint32_t JSTextWrapper::GetMirrorChar(char16_t  c)
{
    EA::WebKit::ITextSystem* pTS = EA::WebKit::GetTextSystem();
    return pTS->GetMirrorChar(c);
}

int32_t JSTextWrapper::GetCombiningClass(char16_t c)
{
    EA::WebKit::ITextSystem* pTS = EA::WebKit::GetTextSystem();
    return pTS->GetCombiningClass(c);
}

uint32_t JSTextWrapper::GetBidiClass(char16_t c)
{
    EA::WebKit::ITextSystem* pTS = EA::WebKit::GetTextSystem();
    return pTS->GetBidiClass(c);
}

uint32_t JSTextWrapper::ConvertCase(const JSText::Char* pTextInput, uint32_t nTextLength, JSText::Char* pTextOutput, uint32_t nTextOutputCapacity, JSText::CaseType caseType)
{
    EA::WebKit::ITextSystem* pTS = EA::WebKit::GetTextSystem(); 
    return pTS->ConvertCase(static_cast<const EA::WebKit::Char*>(pTextInput), nTextLength, static_cast<EA::WebKit::Char*>(pTextOutput), nTextOutputCapacity, static_cast<EA::WebKit::CaseType>(caseType));
}

} // namespace Internal

} // namespace EA







