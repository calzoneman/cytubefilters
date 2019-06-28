#include <v8.h>
#include <nan.h>

#include "./filter.h"
#include "./util.h"

using v8::Boolean;
using v8::Local;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

namespace Util
{
    bool SafeGetObject(const Local<Object>& obj, uint32_t index, Local<Object>& dest)
    {
        Local<Value> value;
        Local<Object> object;

        if (!Nan::Get(obj, index).ToLocal(&value))    return false;
        if (!Nan::To<Object>(value).ToLocal(&object)) return false;
        if (!object->IsObject())                      return false;

        dest = object;
        return true;
    }

    bool SafeGetString(const Local<Object>& obj, const char *key, std::string& dest)
    {
        Local<String> objKey;
        Local<Value> value;

        if (!Nan::New<String>(key).ToLocal(&objKey)) return false;
        if (!Nan::Get(obj, objKey).ToLocal(&value))  return false;
        if (!value->IsString())                      return false;

        dest = *Nan::Utf8String(value);
        return true;
    }

    inline bool SafeSetString(const Local<Object>& obj, const char *key, std::string value)
    {
        Local<String> objKey;
        Local<String> propValue;

        if (!Nan::New<String>(key).ToLocal(&objKey))      return false;
        if (!Nan::New<String>(value).ToLocal(&propValue)) return false;

        Nan::Set(obj, objKey, propValue);
        return true;
    }

    inline bool SafeGetBool(const Local<Object>& obj, const char *key, bool& dest)
    {
        Local<String> objKey;
        Local<Value> value;

        if (!Nan::New<String>(key).ToLocal(&objKey)) return false;
        if (!Nan::Get(obj, objKey).ToLocal(&value))  return false;
        if (!value->IsBoolean())                     return false;

        Nan::Maybe<bool> boolValue = Nan::To<bool>(value);
        if (!boolValue.IsJust()) return false;

        dest = boolValue.FromJust();
        return true;
    }

    inline bool SafeSetBool(const Local<Object>& obj, const char *key, bool value)
    {
        Local<String> objKey;

        if (!Nan::New<String>(key).ToLocal(&objKey)) return false;

        Nan::Set(obj, objKey, Nan::New<Boolean>(value));
        return true;
    }

    bool FromJSObject(const Local<Object>& obj, Filter& out)
    {
        std::string name, source, flags, replacement;
        bool active, filter_links;

        if (!SafeGetString(obj, "name", name))              return false;
        if (!SafeGetString(obj, "source", source))          return false;
        if (!SafeGetString(obj, "flags", flags))            return false;
        if (!SafeGetString(obj, "replace", replacement))    return false;
        if (!SafeGetBool(obj, "active", active))            return false;
        if (!SafeGetBool(obj, "filterlinks", filter_links)) return false;

        out = Filter(name, source, flags, replacement, active, filter_links);
        return true;
    }

    bool ToJSObject(const Filter& src, Local<Object>& dst)
    {
        if (!SafeSetString(dst, "name", src.name()))              return false;
        if (!SafeSetString(dst, "source", src.source()))          return false;
        if (!SafeSetString(dst, "flags", src.flags()))            return false;
        if (!SafeSetString(dst, "replace", src.replacement()))    return false;
        if (!SafeSetBool(dst, "active", src.active()))            return false;
        if (!SafeSetBool(dst, "filterlinks", src.filter_links())) return false;

        return true;
    }
}
