/*
Copyright (C) 2011, 2012 Electronic Arts, Inc.  All rights reserved.

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

#include <EAWebKit/EAWebkitJavascriptValue.h>
#include <internal/include/EAWebKitAssert.h>
#include <internal/include/EAWebkitEASTLHelpers.h>
#include <internal/include/EAWebKitString.h>
#include <internal/include/EAWebKitNewDelete.h>
#include <internal/include/EAWebKitFPUPrecision.h>
#include <EAAssert/eaassert.h>

#include <EASTL/fixed_vector.h>

#include <JSValue.h>
#include <JSFunction.h>
#include <JSDOMBinding.h>
#include <Protect.h>

namespace EA { namespace WebKit 
{
    typedef eastl::fixed_vector<JSC::JSValue, 8> JavascriptArgumentList;

    /////////////////////////////////////////////////////////////////////////////////////
    // Helpers
    /////////////////////////////////////////////////////////////////////////////////////
    namespace 
    {
        JSC::JSArray *ValueToArray(void *v) 
        {
			SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
            JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(v);
            EAW_ASSERT_MSG(jsValue.isObject(), "Incoming value is not an object");

            JSC::JSObject *jsObject = jsValue.getObject();
            EAW_ASSERT_MSG(jsObject->inherits(&JSC::JSArray::s_info), "Incoming value is not an array");

            return static_cast<JSC::JSArray*>(jsObject);
        }

        JSC::JSObject *ValueToObject(void *v) 
        {
			SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
            JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(v);
            EAW_ASSERT_MSG(jsValue.isObject(),"Incoming value is not an object");

            return jsValue.getObject();
        }

        JSC::JSFunction *ValueToFunction(void *v)
        {
			SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
            JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(v);
            EAW_ASSERT_MSG(jsValue.isObject(),"Incoming value is not an object");

            JSC::JSObject *jsObject = jsValue.getObject();
            EAW_ASSERT_MSG(jsObject->inherits(&JSC::JSFunction::s_info),"Incoming value is not a function");

            return static_cast<JSC::JSFunction*>(jsObject);
        }
    }


    /////////////////////////////////////////////////////////////////////////////////////
    // JavascriptValue
    /////////////////////////////////////////////////////////////////////////////////////
    EA_COMPILETIME_ASSERT(JavascriptValue::kJSValueSize == sizeof(JSC::JSValue));
#if defined(EA_PLATFORM_OSX)
    // JSC::JSValue has 4 bytes alignment on OS X. Since the buffer on which we do "in-place" construction is 8 byte aligned (meaning that it is definitely 4 byte aligned),
    // it is safe to compare the alignment check against 4 byte for this class on OS X.
    EA_COMPILETIME_ASSERT(4 == EA_ALIGN_OF(JSC::JSValue));
#else
    EA_COMPILETIME_ASSERT(8 == EA_ALIGN_OF(JSC::JSValue));
#endif
    // Null / undefined
    /////////////////////////////////////////////////////////////////////////////////////
    void JavascriptValue::SetNull(void) 
    {
        JSC::JSValue jsVal = JSC::jsNull();
        Assign(&jsVal);
    }

    void JavascriptValue::SetUndefined(void) 
    {
        JSC::JSValue jsVal = JSC::jsUndefined();
        Assign(&jsVal);
    }

    // Numbers
    /////////////////////////////////////////////////////////////////////////////////////
    void JavascriptValue::SetNumberValue(double v) 
    {
		SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
        JSC::JSValue jsVal = JSC::jsNumber(v);
        Assign(&jsVal);
    }

    void JavascriptValue::SetNumberValue(int v)
    {
        JSC::JSValue jsVal = JSC::jsNumber(v);
        Assign(&jsVal);
    }
    
    double JavascriptValue::GetNumberValue(void) const 
    {
		SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
        JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(GetImpl());
        EAW_ASSERT_MSG(jsValue.isNumber(),"Trying to read number value from a JavaScriptValue which is not a number");

        double number = 0.0f;
        jsValue.getNumber(number);
        return number;
    }

    bool JavascriptValue::IsInt32(void) const 
    {
        JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(GetImpl());
        return jsValue.isInt32();
    }

    int JavascriptValue::GetInt32(void) const 
    {
        JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(GetImpl());
        return jsValue.asInt32();
    }

    bool JavascriptValue::IsDouble(void) const 
    {
        JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(GetImpl());
        return jsValue.isDouble();
    }

    double JavascriptValue::GetDouble(void) const 
    {
        JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(GetImpl());
        return jsValue.asDouble();
    }

    // Booleans
    /////////////////////////////////////////////////////////////////////////////////////
    void JavascriptValue::SetBooleanValue(bool v) 
    {
        JSC::JSValue jsVal = JSC::jsBoolean(v);
        Assign(&jsVal);
    }

    bool JavascriptValue::GetBooleanValue(void) const 
    {
        JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(GetImpl());
        EAW_ASSERT_MSG(jsValue.isBoolean(),"Trying to read boolean value from a JavaScriptValue which is not a boolean");

        return jsValue.getBoolean();
    }

    // Strings
    /////////////////////////////////////////////////////////////////////////////////////
    void JavascriptValue::SetStringValue(const char16_t *v) 
    {
        JSC::UString s(v);
        JSC::JSValue jsVal = JSC::jsString(reinterpret_cast<JSC::ExecState*>(mExecState), s);
        Assign(&jsVal);
    }

    const char16_t *JavascriptValue::GetStringValue(size_t *pLengthOut) const 
    {
        JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(GetImpl());
        EAW_ASSERT_MSG(jsValue.isString(),"Trying to read characters from a JavaScriptValue which is not a string");

        JSC::UString s = jsValue.getString(reinterpret_cast<JSC::ExecState*>(mExecState));
        *pLengthOut = s.length();
        return s.characters();
    }

    // Arrays
    /////////////////////////////////////////////////////////////////////////////////////
    void JavascriptValue::SetArrayType(size_t arraySize) 
    {
        JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(mExecState);
        JSC::JSValue jsVal(JSC::constructEmptyArray(exec, arraySize));
        Assign(&jsVal);
    }

    void JavascriptValue::SetArrayValue(size_t index, const JavascriptValue &v) 
    {
        EAW_ASSERT(v.mJSValue != NULL);

        JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(mExecState);

        JSC::JSArray *jsArray = ValueToArray(GetImpl());
        jsArray->put(exec, index, *reinterpret_cast<JSC::JSValue*>(v.GetImpl()));
    }

    void JavascriptValue::SetArrayValues(const JavascriptValue *array, size_t count, size_t startIndex)
    {
        EAW_ASSERT(array != NULL);

        JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(mExecState);
        JSC::JSArray *jsArray = ValueToArray(GetImpl());
        
        for (size_t i = 0; i < count; ++i)
        {   
            jsArray->put(exec, startIndex + i, *reinterpret_cast<JSC::JSValue*>(array[i].GetImpl()));
        }
    }

    void JavascriptValue::PushArrayValue(const JavascriptValue &v) 
    {
        EAW_ASSERT(v.mJSValue != NULL);
        
        JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(v.GetImpl());
        JSC::JSArray *jsArray = ValueToArray(GetImpl());
        JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(mExecState);

        jsArray->push(exec, jsValue);
    }

    void JavascriptValue::PopArrayValue(JavascriptValue *pValueOut) 
    {
        JSC::JSArray *jsArray = ValueToArray(GetImpl());
        JSC::JSValue back = jsArray->pop();

        *pValueOut = JavascriptValue(&back, mExecState);
    }

    void JavascriptValue::GetArrayValue(size_t index, JavascriptValue* pValueOut) const 
    {
        JSC::JSArray *jsArray = ValueToArray(GetImpl());
        
        JSC::JSValue atIndex = jsArray->getIndex(index);
        *pValueOut = JavascriptValue(&atIndex, mExecState);
    }

    size_t JavascriptValue::GetArrayLength(void) const 
    {
        JSC::JSArray *jsArray = ValueToArray(GetImpl());
        return jsArray->length();
    }

    // Objects
    /////////////////////////////////////////////////////////////////////////////////////
    void JavascriptValue::SetObjectType() 
    {
        JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(mExecState);
        JSC::JSValue jsVal(JSC::constructEmptyObject(exec));
        Assign(&jsVal);
    }

    void JavascriptValue::SetProperty(const char16_t *key, const JavascriptValue &v) 
    {
		SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
        JSC::JSObject *jsObject = ValueToObject(GetImpl());
        JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(mExecState);
        JSC::JSValue sourceJSValue = *reinterpret_cast<JSC::JSValue*>(v.GetImpl());

        JSC::Identifier id(exec, key, EA::Internal::Strlen(key));
        JSC::PutPropertySlot slot;
        jsObject->put(exec, id, sourceJSValue, slot);
    }

    void JavascriptValue::GetProperty(const char16_t *key, JavascriptValue* pValueOut) const 
    {
		SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
        JSC::JSObject *jsObject = ValueToObject(GetImpl());
        JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(mExecState);

        JSC::Identifier id(exec, key, EA::Internal::Strlen(key));
        JSC::JSValue jsValue = jsObject->get(exec, id);
        
        *pValueOut = JavascriptValue(&jsValue, exec);
    }

    JavascriptValue::PropertyIterator* JavascriptValue::First(void) 
    {
        JSC::JSObject *jsObject = ValueToObject(GetImpl()); // This asserts that the object is a JSObject.

        if (mIterator == NULL) 
        {
            mIterator = EAWEBKIT_NEW("JavascriptValuePropertyIterator") JavascriptValue::PropertyIterator(this);  
        }

        JSC::PropertyNameArray *names = reinterpret_cast<JSC::PropertyNameArray*>((void*)mIterator->mNames);
        JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(mExecState);
        jsObject->getPropertyNames(exec, *names);

        if (names->size() == 0) 
        {
            return NULL;
        }

        mIterator->mIterator = (void*)names->begin();
        return mIterator;
    }

    JavascriptValue::PropertyIterator* JavascriptValue::GetNext(void) 
    {
        if (mIterator == NULL) 
        {
            return NULL;
        }

        JSC::PropertyNameArray::const_iterator iProperty = reinterpret_cast<JSC::PropertyNameArray::const_iterator>(mIterator->mIterator);
        if (iProperty == NULL) 
        {
            return NULL;
        }

        ++iProperty;
        
        JSC::PropertyNameArray *names = reinterpret_cast<JSC::PropertyNameArray*>((void*)mIterator->mNames);
        if (iProperty == names->end()) 
        {
            mIterator->mIterator = NULL;
            return NULL;
        }

        mIterator->mIterator = (void*)iProperty;
        return mIterator;
    }

    // Functions
    /////////////////////////////////////////////////////////////////////////////////////
    bool JavascriptValue::Call(JavascriptValue *args, size_t argCount, JavascriptValue *pValueOut)
    {
		SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
        JSC::MarkedArgumentBuffer jsArgs;

        if (args)
        {
            for (size_t i = 0; i < argCount; ++i)
            {
                jsArgs.append(*reinterpret_cast<JSC::JSValue*>(args[i].GetImpl()));
            }
        }
        
        JSC::JSFunction *jsFunction = ValueToFunction(GetImpl());
        JSC::CallData callData;
        JSC::CallType callType = jsFunction->getCallData(callData);
        if (callType != JSC::CallTypeNone)
        {
            JSC::JSValue jsvFunction = *reinterpret_cast<JSC::JSValue*>(GetImpl());
            JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(mExecState);
            JSC::JSValue result = JSC::call(exec, jsvFunction, callType, callData, jsvFunction, jsArgs);

            if (!exec->hadException())
            {
                if (pValueOut)
                {
                    pValueOut->SetExec(mExecState);
                    pValueOut->Assign(&result);
                }

                return true;
            }
            else
            {
                WebCore::reportException(exec, exec->exception());
            }
        }

        if (pValueOut)
        {
            pValueOut->SetUndefined();
        }

        return false;
    }

    // Copying etc.
    /////////////////////////////////////////////////////////////////////////////////////
    JavascriptValue::JavascriptValue(void *jsValue, void *execState)
    : mExecState(execState)
    , mIterator(NULL) 
    {
        new (mJSValue) JSC::JSValue();
        Assign(jsValue);
    }

    JavascriptValue::JavascriptValue(void)
    : mExecState(NULL)
    , mIterator(NULL) 
    {
        new (mJSValue) JSC::JSValue();
    }

    JavascriptValue::JavascriptValue(const JavascriptValue &source) 
    {
        mIterator = NULL;
        new (mJSValue) JSC::JSValue();
        Copy(source);
    }

    void JavascriptValue::Destruct(void) 
    {
        if (mIterator) 
        {
            EAWEBKIT_DELETE mIterator;
            mIterator = NULL;
        }
        
        JSC::JSValue *jsValue = reinterpret_cast<JSC::JSValue*>(GetImpl());
        JSC::gcUnprotect(*jsValue);
        jsValue->~JSValue();
    }

    void JavascriptValue::Copy(const JavascriptValue &source) 
    {
        const JSC::JSValue *sourceJSValue = reinterpret_cast<const JSC::JSValue*>(source.GetImpl());

        Assign(sourceJSValue);
        mExecState = source.mExecState;
    }

    void JavascriptValue::Assign(const void *jsValue) 
    {
        const JSC::JSValue *value = reinterpret_cast<const JSC::JSValue*>(jsValue);
        JSC::JSValue *thisValue = reinterpret_cast<JSC::JSValue*>(GetImpl());
        JSC::gcUnprotect(*thisValue);
        *thisValue = *value;
        JSC::gcProtect(*thisValue);
    }

    JavascriptValueType::JavascriptValueType JavascriptValue::Type(void) const 
    {
        JSC::JSValue jsValue = *reinterpret_cast<JSC::JSValue*>(GetImpl());
        if (jsValue.isNumber()) 
        {
            return JavascriptValueType::Number;
        }
        else if (jsValue.isBoolean()) 
        {
            return JavascriptValueType::Boolean;
        }
        else if (jsValue.isString()) 
        {
            return JavascriptValueType::StringType;
        }
        else if (jsValue.isObject()) 
        {
            JSC::JSObject *jsObject = jsValue.getObject();
            if (jsObject->inherits(&JSC::JSArray::s_info)) 
            {
                return JavascriptValueType::Array;
            }
            else if (jsObject->inherits(&JSC::JSFunction::s_info))
            {
                return JavascriptValueType::Function;
            }
            else 
            {
                return JavascriptValueType::Object;
            }
        }
        else if (jsValue.isUndefined()) 
        {
            return JavascriptValueType::Undefined;
        }
        else if (jsValue.isNull()) 
        {
            return JavascriptValueType::Null;
        }
        else 
        {
            return JavascriptValueType::Unknown;
        }
    }

    void *JavascriptValue::GetImpl(void) const 
    {
        return (void*)&mJSValue;
    }

    void JavascriptValue::SetExec(void *exec) 
    {
        mExecState = exec;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // JavascriptValue::PropertyIterator
    /////////////////////////////////////////////////////////////////////////////////////
    EA_COMPILETIME_ASSERT(JavascriptValue::PropertyIterator::kPropertyNameArraySize >= sizeof(JSC::PropertyNameArray));
    EA_COMPILETIME_ASSERT(EA_PLATFORM_PTR_SIZE == EA_ALIGN_OF(JSC::PropertyNameArray));

    JavascriptValue::PropertyIterator::PropertyIterator(JavascriptValue *owner) 
    {
        mObject = owner;
        mIterator = NULL;

        JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(owner->mExecState);
        new (mNames) JSC::PropertyNameArray(exec);
    }

    JavascriptValue::PropertyIterator::~PropertyIterator(void) 
    {
        JSC::PropertyNameArray *names = reinterpret_cast<JSC::PropertyNameArray*>((void*)mNames);
        names->~PropertyNameArray();
        mIterator = NULL;
    }

    void JavascriptValue::PropertyIterator::GetValue(JavascriptValue* pValueOut) const 
    {
		SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
        JSC::PropertyNameArray *names = reinterpret_cast<JSC::PropertyNameArray*>((void*)mNames);
        JSC::PropertyNameArray::const_iterator iProperty = reinterpret_cast<JSC::PropertyNameArray::const_iterator>(mIterator);
        EAW_ASSERT(iProperty != names->end());
        (void) names;

        JSC::JSObject *jsObject = ValueToObject(mObject->GetImpl());
        JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(mObject->mExecState);

        JSC::JSValue jsValue = jsObject->get(exec, *iProperty);
        *pValueOut = JavascriptValue(&jsValue, exec);
    }

    const char16_t *JavascriptValue::PropertyIterator::GetKey(size_t *pLengthOut) const 
    {
        JSC::PropertyNameArray *names = reinterpret_cast<JSC::PropertyNameArray*>((void*)mNames);
        JSC::PropertyNameArray::const_iterator iProperty = reinterpret_cast<JSC::PropertyNameArray::const_iterator>(mIterator);
        EAW_ASSERT(iProperty != names->end());
        (void) names;

        *pLengthOut = iProperty->length();
        return iProperty->characters();
    }

    void JavascriptValue::PropertyIterator::SetValue(const JavascriptValue &v) 
    {
		SET_AUTOFPUPRECISION(EA::WebKit::kFPUPrecisionExtended);
        JSC::PropertyNameArray *names = reinterpret_cast<JSC::PropertyNameArray*>((void*)mNames);
        JSC::PropertyNameArray::const_iterator iProperty = reinterpret_cast<JSC::PropertyNameArray::const_iterator>(mIterator);
        EAW_ASSERT(iProperty != names->end());
        (void) names;
        JSC::JSObject *jsObject = ValueToObject(mObject->GetImpl());
        JSC::ExecState *exec = reinterpret_cast<JSC::ExecState*>(mObject->mExecState);

        JSC::PutPropertySlot slot;
        jsObject->put(exec, *iProperty, *reinterpret_cast<JSC::JSValue*>(v.GetImpl()), slot);
    }
}}
