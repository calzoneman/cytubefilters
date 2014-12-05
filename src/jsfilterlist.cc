#include <node.h>
#include <nan.h>
#include <pcrecpp.h>
#include <sstream>

#include "./jsfilterlist.h"
#include "./filterlist.h"
#include "./filter.h"
#include "./util.h"

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

    for (uint32_t i = 0; i < indexes->Length(); i++)
    {
        if (!filters->Get(i)->IsObject())
        {
            std::ostringstream oss;
            oss << "Filter at index " << i << " is not an object";
            return NanThrowTypeError(oss.str().c_str());
        }

        Local<Object> f = filters->Get(i)->ToObject();
        if (!Util::ValidFilter(f))
        {
            std::ostringstream oss;
            oss << "Filter at index " << i << " is invalid";
            return NanThrowTypeError(oss.str().c_str());
        }

        filter_list.add_filter(Util::NewFilter(f));
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

    std::vector<Filter>::iterator it;
    for (it = filters.begin(); it < filters.end(); it++, i++)
    {
        Local<Object> filter = NanNew<Object>();
        Util::PackFilter(*it, filter);

        result->Set(i, filter);
    }

    NanReturnValue(result);
}

NAN_METHOD(JSFilterList::UpdateFilter)
{
    NanScope();

    if (args.Length() < 1)
    {
        return NanThrowError("UpdateFilter expects 1 argument");
    }

    if (!args[0]->IsObject())
    {
        return NanThrowTypeError("Filter to be upated must be an object");
    }

    Local<Object> obj = args[0]->ToObject();

    std::string name = *String::Utf8Value(obj->Get(Util::NameField)->ToString());
    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(args.This());
    Filter *filter = wrap->m_FilterList.find_filter(name);

    if (filter == NULL)
    {
        return NanThrowError("Filter to be updated does not exist");
    }

    Local<Array> fields = obj->GetPropertyNames();
    for (unsigned int i = 0; i < fields->Length(); i++)
    {
        Local<String> field = fields->Get(i)->ToString();
        std::string sfield = *String::Utf8Value(field);

        if (sfield == "source")
        {
            filter->set_source(*String::Utf8Value(obj->Get(field)->ToString()));
        }
    }

    NanReturnValue(obj);
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
    tpl->InstanceTemplate()->Set(NanNew<String>("updateFilter"),
        FunctionTemplate::New(JSFilterList::UpdateFilter));
}

void Init(Handle<Object> exports, Handle<Object> module)
{
    JSFilterList::Init();
    Util::Init();
    Local<FunctionTemplate> constructor_handle = NanNew(constructor);

    module->Set(NanNew<String>("exports"), constructor_handle->GetFunction());
}

NODE_MODULE(cytubefilters, Init);
