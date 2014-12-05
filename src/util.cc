#include <v8.h>
#include <nan.h>

#include "./filter.h"
#include "./util.h"

using v8::Boolean;
using v8::Local;
using v8::Object;
using v8::Persistent;
using v8::String;

static Persistent<String> name_field, source_field, flags_field,
    replace_field, active_field, filter_links_field;

namespace Util
{

    bool ValidFilter(const Local<Object>& obj)
    {
        if (!obj->Get(name_field)->IsString() ||
            !obj->Get(source_field)->IsString() ||
            !obj->Get(flags_field)->IsString() ||
            !obj->Get(replace_field)->IsString() ||
            !obj->Get(active_field)->IsBoolean() ||
            !obj->Get(filter_links_field)->IsBoolean())
        {
            return false;
        }

        return true;
    }

    Filter NewFilter(const Local<Object>& obj)
    {
        std::string name(*String::Utf8Value(obj->Get(name_field)->ToString()));
        std::string source(*String::Utf8Value(obj->Get(source_field)->ToString()));
        std::string flags(*String::Utf8Value(obj->Get(flags_field)->ToString()));
        std::string replacement(*String::Utf8Value(obj->Get(replace_field)->ToString()));
        bool active = obj->Get(active_field)->BooleanValue();
        bool filter_links = obj->Get(filter_links_field)->BooleanValue();

        Filter filter(name, source, flags, replacement, active, filter_links);
        return filter;
    }

    void PackFilter(const Filter& src, Local<Object>& dst)
    {
        dst->Set(name_field         , NanNew<String>(src.name()));
        dst->Set(source_field       , NanNew<String>(src.source()));
        dst->Set(flags_field        , NanNew<String>(src.flags()));
        dst->Set(replace_field      , NanNew<String>(src.replacement()));
        dst->Set(active_field       , NanNew<Boolean>(src.active()));
        dst->Set(filter_links_field , NanNew<Boolean>(src.filter_links()));
    }

    void Init()
    {
        name_field           = Persistent<String>::New(NanNew<String>("name"));
        source_field         = Persistent<String>::New(NanNew<String>("source"));
        flags_field          = Persistent<String>::New(NanNew<String>("flags"));
        replace_field        = Persistent<String>::New(NanNew<String>("replace"));
        active_field         = Persistent<String>::New(NanNew<String>("active"));
        filter_links_field   = Persistent<String>::New(NanNew<String>("filterlinks"));
    }
}
