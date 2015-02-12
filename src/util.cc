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
        if (!obj->Get(NanNew<String>(NameField))->IsString() ||
            !obj->Get(NanNew<String>(SourceField))->IsString() ||
            !obj->Get(NanNew<String>(FlagsField))->IsString() ||
            !obj->Get(NanNew<String>(ReplaceField))->IsString() ||
            !obj->Get(NanNew<String>(ActiveField))->IsBoolean() ||
            !obj->Get(NanNew<String>(FilterLinksField))->IsBoolean())
        {
            return false;
        }

        return true;
    }

    Filter NewFilter(const Local<Object>& obj)
    {
        std::string name(*String::Utf8Value(
                obj->Get(NanNew<String>(NameField))->ToString()));
        std::string source(*String::Utf8Value(
                obj->Get(NanNew<String>(SourceField))->ToString()));
        std::string flags(*String::Utf8Value(
                obj->Get(NanNew<String>(FlagsField))->ToString()));
        std::string replacement(*String::Utf8Value(
                obj->Get(NanNew<String>(ReplaceField))->ToString()));
        bool active = obj->Get(NanNew<String>(ActiveField))->BooleanValue();
        bool filter_links = obj->Get(NanNew<String>(FilterLinksField))->BooleanValue();

        Filter filter(name, source, flags, replacement, active, filter_links);
        return filter;
    }

    void PackFilter(const Filter& src, Local<Object>& dst)
    {
        dst->Set(NanNew<String>(NameField)         , NanNew<String>(src.name()));
        dst->Set(NanNew<String>(SourceField)       , NanNew<String>(src.source()));
        dst->Set(NanNew<String>(FlagsField)        , NanNew<String>(src.flags()));
        dst->Set(NanNew<String>(ReplaceField)      , NanNew<String>(src.replacement()));
        dst->Set(NanNew<String>(ActiveField)       , NanNew<Boolean>(src.active()));
        dst->Set(NanNew<String>(FilterLinksField)  , NanNew<Boolean>(src.filter_links()));
    }

    void Init()
    {
        NanAssignPersistent(NameField, NanNew<String>("name"));
        NanAssignPersistent(SourceField, NanNew<String>("source"));
        NanAssignPersistent(FlagsField, NanNew<String>("flags"));
        NanAssignPersistent(ReplaceField, NanNew<String>("replace"));
        NanAssignPersistent(ActiveField, NanNew<String>("active"));
        NanAssignPersistent(FilterLinksField, NanNew<String>("filterlinks"));
    }
}
