#include <v8.h>
#include <nan.h>

#include "./filter.h"
#include "./util.h"

using v8::Boolean;
using v8::Local;
using v8::Object;
using v8::Persistent;
using v8::String;

namespace Util
{
    Persistent<String> NameField, SourceField, FlagsField,
        ReplaceField, ActiveField, FilterLinksField;

    bool ValidFilter(const Local<Object>& obj)
    {
        if (!obj->Get(NameField)->IsString() ||
            !obj->Get(SourceField)->IsString() ||
            !obj->Get(FlagsField)->IsString() ||
            !obj->Get(ReplaceField)->IsString() ||
            !obj->Get(ActiveField)->IsBoolean() ||
            !obj->Get(FilterLinksField)->IsBoolean())
        {
            return false;
        }

        return true;
    }

    Filter NewFilter(const Local<Object>& obj)
    {
        std::string name(*String::Utf8Value(obj->Get(NameField)->ToString()));
        std::string source(*String::Utf8Value(obj->Get(SourceField)->ToString()));
        std::string flags(*String::Utf8Value(obj->Get(FlagsField)->ToString()));
        std::string replacement(*String::Utf8Value(obj->Get(ReplaceField)->ToString()));
        bool active = obj->Get(ActiveField)->BooleanValue();
        bool filter_links = obj->Get(FilterLinksField)->BooleanValue();

        Filter filter(name, source, flags, replacement, active, filter_links);
        return filter;
    }

    void PackFilter(const Filter& src, Local<Object>& dst)
    {
        dst->Set(NameField         , NanNew<String>(src.name()));
        dst->Set(SourceField       , NanNew<String>(src.source()));
        dst->Set(FlagsField        , NanNew<String>(src.flags()));
        dst->Set(ReplaceField      , NanNew<String>(src.replacement()));
        dst->Set(ActiveField       , NanNew<Boolean>(src.active()));
        dst->Set(FilterLinksField  , NanNew<Boolean>(src.filter_links()));
    }

    void Init()
    {
        NameField           = Persistent<String>::New(NanNew<String>("name"));
        SourceField         = Persistent<String>::New(NanNew<String>("source"));
        FlagsField          = Persistent<String>::New(NanNew<String>("flags"));
        ReplaceField        = Persistent<String>::New(NanNew<String>("replace"));
        ActiveField         = Persistent<String>::New(NanNew<String>("active"));
        FilterLinksField    = Persistent<String>::New(NanNew<String>("filterlinks"));
    }
}
