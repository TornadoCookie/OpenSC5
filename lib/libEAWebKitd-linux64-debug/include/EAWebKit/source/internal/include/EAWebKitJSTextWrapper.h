/*
Copyright (C) 2011, 2013 Electronic Arts, Inc.  All rights reserved.

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


#ifndef EAWEBITJSTEXTWRAPPER_H
#define EAWEBITJSTEXTWRAPPER_H

#include <string.h>
#include <EABase/eabase.h>
#include <JavaScriptCore/API/JSTextInterface.h>


namespace EA
{
    namespace Internal
    {
        class JSTextWrapper : public JSTextInterface
        {
        public:           
            virtual ~JSTextWrapper() {}    
            virtual uint32_t GetCharCategory(char16_t c);          
            virtual uint32_t GetMirrorChar(char16_t  c); 
            virtual int32_t GetCombiningClass(char16_t c);
            virtual uint32_t GetBidiClass(char16_t c);
            virtual uint32_t ConvertCase(const JSText::Char* pTextInput, uint32_t nTextLength, JSText::Char* pTextOutput, uint32_t nTextOutputCapacity, JSText::CaseType caseType);
        };
    }
}
#endif // Header include guard

