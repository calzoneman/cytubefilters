#include <node.h>
#include <nan.h>
#include <pcrecpp.h>
#include <sstream>

#include "./jsfilterlist.h"
#include "./filterlist.h"
#include "./filter.h"

using v8::Handle;
using v8::Array;
using v8::Object;
using v8::String;
using v8::Persistent;
using v8::FunctionTemplate;
using v8::Local;
using v8::Boolean;

static Persistent<FunctionTemplate> constructor;

JSFilterList::JSFilterList(const FilterList& filter_list) : m_FilterList(filter_list)
{
}

JSFilterList::~JSFilterList()
{
}

NAN_METHOD(JSFilterList::New)
{
    NanScope();

    JSFilterList *wrap;
    FilterList filter_list;
    if (args.Length() == 0)
    {
        wrap = new JSFilterList(filter_list);
        wrap->Wrap(args.This());
        NanReturnThis();
    }

    if (!args[0]->IsArray())
    {
        return NanThrowTypeError("Argument to FilterList constructor must be an array");
    }

    Local<Object> filters = args[0]->ToObject();
    Local<Array> indexes = filters->GetPropertyNames();

    Local<String> name_field = NanNew<String>("name"),
        source_field         = NanNew<String>("source"),
        flags_field          = NanNew<String>("flags"),
        replace_field        = NanNew<String>("replace"),
        active_field         = NanNew<String>("active"),
        filter_links_field   = NanNew<String>("filterlinks");

    for (uint32_t i = 0; i < indexes->Length(); i++)
    {
        if (!filters->Get(i)->IsObject())
        {
            std::ostringstream oss;
            oss << "Filter at index " << i << " is not an object";
            return NanThrowTypeError(oss.str().c_str());
        }

        Local<Object> f = filters->Get(i)->ToObject();
        if (!f->Get(name_field)->IsString() ||
            !f->Get(source_field)->IsString() ||
            !f->Get(flags_field)->IsString() ||
            !f->Get(replace_field)->IsString() ||
            !f->Get(active_field)->IsBoolean() ||
            !f->Get(filter_links_field)->IsBoolean())
        {
            std::ostringstream oss;
            oss << "Filter at index " << i << " is invalid";
            return NanThrowTypeError(oss.str().c_str());
        }
        std::string name(*String::Utf8Value(f->Get(name_field)->ToString()));
        std::string source(*String::Utf8Value(f->Get(source_field)->ToString()));
        std::string flags(*String::Utf8Value(f->Get(flags_field)->ToString()));
        std::string replacement(*String::Utf8Value(f->Get(replace_field)->ToString()));
        bool active = f->Get(active_field)->BooleanValue();
        bool filter_links = f->Get(filter_links_field)->BooleanValue();

        Filter filter(name, source, flags, replacement, active, filter_links);
        filter_list.add_filter(filter);
    }

    wrap = new JSFilterList(filter_list);
    wrap->Wrap(args.This());

    NanReturnThis();
}

NAN_METHOD(JSFilterList::FilterString)
{
    NanScope();

    std::string input = *String::Utf8Value(args[0]->ToString());
    bool filter_links = args[1]->BooleanValue();

    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(args.This());

    wrap->m_FilterList.exec(&input, filter_links);

    NanReturnValue(NanNew<String>(input));
}

NAN_METHOD(JSFilterList::Pack)
{
    NanScope();

    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(args.This());
    std::vector<Filter> filters = wrap->m_FilterList.filters();
    Local<Array> result = NanNew<Array>();
    unsigned int i = 0;

    Local<String> name_field = NanNew<String>("name"),
        source_field         = NanNew<String>("source"),
        flags_field          = NanNew<String>("flags"),
        replace_field        = NanNew<String>("replace"),
        active_field         = NanNew<String>("active"),
        filter_links_field   = NanNew<String>("filterlinks");


    std::vector<Filter>::iterator it;
    for (it = filters.begin(); it < filters.end(); it++, i++)
    {
        Local<Object> filter = NanNew<Object>();
        filter->Set(name_field         , NanNew<String>(it->name()));
        filter->Set(source_field       , NanNew<String>(it->source()));
        filter->Set(flags_field        , NanNew<String>(it->flags()));
        filter->Set(replace_field      , NanNew<String>(it->replacement()));
        filter->Set(active_field       , NanNew<Boolean>(it->active()));
        filter->Set(filter_links_field , NanNew<Boolean>(it->filter_links()));

        result->Set(i, filter);
    }

    NanReturnValue(result);
}

NAN_METHOD(JSFilterList::QuoteMeta)
{
    NanScope();

    std::string quoted = pcrecpp::RE::QuoteMeta(*String::Utf8Value(args[0]->ToString()));

    NanReturnValue(NanNew<String>(quoted));
}

void JSFilterList::Init()
{
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(JSFilterList::New);
    NanAssignPersistent(constructor, tpl);
    tpl->SetClassName(NanNew<String>("FilterList"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->Set(NanNew<String>("quoteMeta"),
        FunctionTemplate::New(JSFilterList::QuoteMeta));

    tpl->InstanceTemplate()->Set(NanNew<String>("filter"),
        FunctionTemplate::New(JSFilterList::FilterString));
    tpl->InstanceTemplate()->Set(NanNew<String>("pack"),
        FunctionTemplate::New(JSFilterList::Pack));
}

void Init(Handle<Object> exports, Handle<Object> module)
{
    JSFilterList::Init();
    Local<FunctionTemplate> constructor_handle = NanNew(constructor);

    module->Set(NanNew<String>("exports"), constructor_handle->GetFunction());
}

NODE_MODULE(cytubefilters, Init);
