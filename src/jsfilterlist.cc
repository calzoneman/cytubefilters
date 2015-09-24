#include <node.h>
#include <nan.h>
#include <pcrecpp.h>
#include <sstream>

#include "./jsfilterlist.h"
#include "./filterlist.h"
#include "./filter.h"
#include "./util.h"

using v8::Array;
using v8::Boolean;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

static Nan::Persistent<FunctionTemplate> constructor;

JSFilterList::JSFilterList(const FilterList& filter_list) : m_FilterList(filter_list)
{
}

JSFilterList::~JSFilterList()
{
}

NAN_METHOD(JSFilterList::New)
{
    Nan::HandleScope scope;

    JSFilterList *wrap;
    FilterList filter_list;
    if (info.Length() == 0)
    {
        wrap = new JSFilterList(filter_list);
        wrap->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        return;
    }

    if (!info[0]->IsArray())
    {
        Nan::ThrowTypeError("Argument to FilterList constructor must be an array");
        return;
    }

    Local<Object> filters = info[0]->ToObject();
    Local<Array> indexes = filters->GetPropertyNames();

    for (uint32_t i = 0; i < indexes->Length(); i++)
    {
        if (!filters->Get(i)->IsObject())
        {
            std::ostringstream oss;
            oss << "Filter at index " << i << " is not an object";
            Nan::ThrowTypeError(oss.str().c_str());
            return;
        }

        Local<Object> f = filters->Get(i)->ToObject();
        if (!Util::ValidFilter(f))
        {
            std::ostringstream oss;
            oss << "Filter at index " << i << " is invalid";
            Nan::ThrowTypeError(oss.str().c_str());
            return;
        }

        filter_list.add_filter(Util::NewFilter(f));
    }

    wrap = new JSFilterList(filter_list);
    wrap->Wrap(info.This());

    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(JSFilterList::FilterString)
{
    Nan::HandleScope scope;

    std::string input = *Nan::Utf8String(info[0]->ToString());
    bool filter_links = info[1]->BooleanValue();

    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(info.This());

    wrap->m_FilterList.exec(&input, filter_links);

    info.GetReturnValue().Set(Nan::New<String>(input).ToLocalChecked());
}

NAN_METHOD(JSFilterList::Pack)
{
    Nan::HandleScope scope;

    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(info.This());
    std::vector<Filter> filters = wrap->m_FilterList.filters();
    Local<Array> result = Nan::New<Array>();
    unsigned int i = 0;

    std::vector<Filter>::iterator it;
    for (it = filters.begin(); it < filters.end(); it++, i++)
    {
        Local<Object> filter = Nan::New<Object>();
        Util::PackFilter(*it, filter);

        Nan::Set(result, i, filter);
    }

    info.GetReturnValue().Set(result);
}

NAN_METHOD(JSFilterList::UpdateFilter)
{
    Nan::HandleScope scope;

    if (info.Length() < 1)
    {
        Nan::ThrowError("updateFilter expects 1 argument");
        return;
    }

    if (!info[0]->IsObject())
    {
        Nan::ThrowTypeError("Filter to be updated must be an object");
        return;
    }

    Local<Object> obj = info[0]->ToObject();

    std::string name = *Nan::Utf8String(
            Nan::Get(obj, Nan::New<String>("name").ToLocalChecked())
                .ToLocalChecked()->ToString());
    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(info.This());
    Filter *filter = wrap->m_FilterList.find_filter(name);

    if (filter == NULL)
    {
        Nan::ThrowError("Filter to be updated does not exist");
        return;
    }

    Local<Array> fields = obj->GetPropertyNames();
    for (unsigned int i = 0; i < fields->Length(); i++)
    {
        Local<String> field = Nan::Get(fields, i).ToLocalChecked()->ToString();
        std::string sfield = *Nan::Utf8String(field);
        Local<Value> value = Nan::Get(obj, field).ToLocalChecked();

        if (!value->IsString() && (sfield == "source" || sfield == "replace" ||
            sfield == "flags"))
        {
            Nan::ThrowTypeError(("Field " + sfield + " must be a string").c_str());
            return;
        }
        else if (!value->IsBoolean() && (sfield == "active" || sfield == "filterlinks"))
        {
            Nan::ThrowTypeError(("Field " + sfield + " must be a boolean").c_str());
            return;
        }

        if (sfield == "source")
        {
            filter->set_source(*Nan::Utf8String(value->ToString()));
        }
        else if (sfield == "replace")
        {
            filter->set_replacement(*Nan::Utf8String(value->ToString()));
        }
        else if (sfield == "flags")
        {
            filter->set_flags(*Nan::Utf8String(value->ToString()));
        }
        else if (sfield == "active")
        {
            filter->set_active(value->ToBoolean()->BooleanValue());
        }
        else if (sfield == "filterlinks")
        {
            filter->set_filter_links(value->ToBoolean()->BooleanValue());
        }
    }

    Local<Object> retval = Nan::New<Object>();
    Util::PackFilter(*filter, retval);

    info.GetReturnValue().Set(retval);
}

NAN_METHOD(JSFilterList::RemoveFilter)
{
    Nan::HandleScope scope;

    if (info.Length() < 1)
    {
        Nan::ThrowError("removeFilter expects 1 argument");
        return;
    }

    if (!info[0]->IsObject())
    {
        Nan::ThrowTypeError("Filter to be removed must be an object");
        return;
    }

    Local<Object> obj = info[0]->ToObject();

    std::string name = *Nan::Utf8String(
            Nan::Get(obj, Nan::New<String>("name").ToLocalChecked())
                .ToLocalChecked()->ToString());
    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(info.This());

    Local<Boolean> removed = Nan::New<Boolean>(wrap->m_FilterList.remove_filter(name));
    info.GetReturnValue().Set(removed);
}

NAN_METHOD(JSFilterList::MoveFilter)
{
    Nan::HandleScope scope;

    if (info.Length() != 2)
    {
        Nan::ThrowError("moveFilter expects 2 arguments");
        return;
    }

    if (!info[0]->IsInt32() || !info[1]->IsInt32())
    {
        Nan::ThrowTypeError("Arguments 'from' and 'to' must both be integers");
        return;
    }

    std::vector<Filter>::size_type from = info[0]->Int32Value(),
                                   to = info[1]->Int32Value();

    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(info.This());
    if (from >= wrap->m_FilterList.size() || to >= wrap->m_FilterList.size())
    {
        Nan::ThrowError("Argument out of range");
        return;
    }

    wrap->m_FilterList.move_filter(from, to);
}

NAN_METHOD(JSFilterList::AddFilter)
{
    Nan::HandleScope scope;

    if (info.Length() != 1)
    {
        Nan::ThrowError("addFilter expects 1 argument");
        return;
    }

    if (!info[0]->IsObject())
    {
        Nan::ThrowTypeError("Filter to add must be an object");
        return;
    }

    Local<Object> f = info[0]->ToObject();
    if (!Util::ValidFilter(f))
    {
        Nan::ThrowError("Invalid filter");
        return;
    }

    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(info.This());
    std::string name = *Nan::Utf8String(
            Nan::Get(f, Nan::New<String>("name").ToLocalChecked())
                .ToLocalChecked()->ToString());
    Filter *filter = wrap->m_FilterList.find_filter(name);

    if (filter != NULL)
    {
        Nan::ThrowError(("Filter '" + name + "' already exists.  Please choose a different name").c_str());
        return;
    }

    wrap->m_FilterList.add_filter(Util::NewFilter(f));
}

NAN_PROPERTY_GETTER(JSFilterList::GetLength)
{
    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(info.This());

    info.GetReturnValue().Set(Nan::New<Number>(wrap->m_FilterList.size()));
}

NAN_METHOD(JSFilterList::QuoteMeta)
{
    Nan::HandleScope scope;

    std::string quoted = pcrecpp::RE::QuoteMeta(
            *Nan::Utf8String(info[0]->ToString()));

    info.GetReturnValue().Set(Nan::New<String>(quoted).ToLocalChecked());
}

NAN_METHOD(JSFilterList::CheckValidRegex)
{
    Nan::HandleScope scope;

    pcrecpp::RE re(*Nan::Utf8String(info[0]->ToString()), DEFAULT_FLAGS);
    if (re.error().size() > 0)
    {
        Nan::ThrowError(re.error().c_str());
        return;
    }

    info.GetReturnValue().Set(Nan::True());
}

void JSFilterList::Init()
{
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(JSFilterList::New);
    tpl->SetClassName(Nan::New<String>("FilterList").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->Set(Nan::New<String>("quoteMeta").ToLocalChecked(),
        Nan::New<FunctionTemplate>(JSFilterList::QuoteMeta));
    tpl->Set(Nan::New<String>("checkValidRegex").ToLocalChecked(),
        Nan::New<FunctionTemplate>(JSFilterList::CheckValidRegex));

    tpl->InstanceTemplate()->Set(Nan::New<String>("filter").ToLocalChecked(),
        Nan::New<FunctionTemplate>(JSFilterList::FilterString));
    tpl->InstanceTemplate()->Set(Nan::New<String>("pack").ToLocalChecked(),
        Nan::New<FunctionTemplate>(JSFilterList::Pack));
    tpl->InstanceTemplate()->Set(Nan::New<String>("addFilter").ToLocalChecked(),
        Nan::New<FunctionTemplate>(JSFilterList::AddFilter));
    tpl->InstanceTemplate()->Set(Nan::New<String>("updateFilter").ToLocalChecked(),
        Nan::New<FunctionTemplate>(JSFilterList::UpdateFilter));
    tpl->InstanceTemplate()->Set(Nan::New<String>("removeFilter").ToLocalChecked(),
        Nan::New<FunctionTemplate>(JSFilterList::RemoveFilter));
    tpl->InstanceTemplate()->Set(Nan::New<String>("moveFilter").ToLocalChecked(),
        Nan::New<FunctionTemplate>(JSFilterList::MoveFilter));

    Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New<String>("length").ToLocalChecked(),
        JSFilterList::GetLength);

    constructor.Reset(tpl);
}

void Init(Handle<Object> exports, Handle<Object> module)
{
    JSFilterList::Init();
    Local<FunctionTemplate> constructor_handle = Nan::New(constructor);

    Nan::Set(module, Nan::New<String>("exports").ToLocalChecked(),
            constructor_handle->GetFunction());
}

NODE_MODULE(cytubefilters, Init);
