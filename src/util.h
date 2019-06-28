#pragma once

#include <v8.h>

#include "./filter.h"

using v8::Local;
using v8::Object;
using v8::Persistent;
using v8::String;

namespace Util
{
    bool SafeGetObject(const Local<Object>& obj, uint32_t index, Local<Object>& dest);
    bool SafeGetString(const Local<Object>& obj, const char *key, std::string& dest);
    bool FromJSObject(const Local<Object>& obj, Filter& dest);
    bool ToJSObject(const Filter& src, Local<Object>& dest);
}
