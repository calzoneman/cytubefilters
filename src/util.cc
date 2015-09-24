#include <v8.h>
#include <nan.h>

#include "./filter.h"
#include "./util.h"

using v8::Boolean;
using v8::Local;
using v8::Object;
using v8::Persistent;
using v8::String;

#define LOCAL_GET(obj, key) Nan::Get(obj, Nan::New<String>(key).ToLocalChecked()).ToLocalChecked()
#define LOCAL_SET(obj, key, value) Nan::Set(obj, Nan::New<String>(key).ToLocalChecked(), value)

namespace Util
{
    bool ValidFilter(const Local<Object>& obj)
    {
        if (!LOCAL_GET(obj, "name")->IsString() ||
            !LOCAL_GET(obj, "source")->IsString() ||
            !LOCAL_GET(obj, "flags")->IsString() ||
            !LOCAL_GET(obj, "active")->IsBoolean() ||
            !LOCAL_GET(obj, "filterlinks")->IsBoolean())
        {
            return false;
        }

        return true;
    }

    Filter NewFilter(const Local<Object>& obj)
    {
        std::string name(*Nan::Utf8String(LOCAL_GET(obj, "name")->ToString()));
        std::string source(*Nan::Utf8String(LOCAL_GET(obj, "source")->ToString()));
        std::string flags(*Nan::Utf8String(LOCAL_GET(obj, "flags")->ToString()));
        std::string replacement(*Nan::Utf8String(LOCAL_GET(obj, "replace")->ToString()));
        bool active = LOCAL_GET(obj, "active")->BooleanValue();
        bool filter_links = LOCAL_GET(obj, "filterlinks")->BooleanValue();

        Filter filter(name, source, flags, replacement, active, filter_links);
        return filter;
    }

    void PackFilter(const Filter& src, Local<Object>& dst)
    {
        LOCAL_SET(dst, "name", Nan::New<String>(src.name()).ToLocalChecked());
        LOCAL_SET(dst, "source", Nan::New<String>(src.source()).ToLocalChecked());
        LOCAL_SET(dst, "flags", Nan::New<String>(src.flags()).ToLocalChecked());
        LOCAL_SET(dst, "replace", Nan::New<String>(src.replacement()).ToLocalChecked());
        LOCAL_SET(dst, "active", Nan::New<Boolean>(src.active()));
        LOCAL_SET(dst, "filterlinks", Nan::New<Boolean>(src.filter_links()));
    }
}
