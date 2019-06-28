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

    Local<Object> filters;
    if (!Nan::To<Object>(info[0]).ToLocal(&filters))
    {
        Nan::ThrowTypeError("Could not convert argument to object");
        return;
    }

    Local<Array> indexes;
    if (!Nan::GetPropertyNames(filters).ToLocal(&indexes))
    {
        Nan::ThrowTypeError("Could not get array indexes");
        return;
    }

    for (uint32_t i = 0; i < indexes->Length(); i++)
    {
        Local<Object> f;
        if (!Util::SafeGetObject(filters, i, f))
        {
            std::ostringstream oss;
            oss << "Filter at index " << i << " is not an object";
            Nan::ThrowTypeError(oss.str().c_str());
            return;
        }
        else
        {
            Filter filter;
            if (!Util::FromJSObject(f, filter))
            {
                std::ostringstream oss;
                oss << "Filter at index " << i << " is invalid";
                Nan::ThrowTypeError(oss.str().c_str());
                return;
            }

            filter_list.add_filter(filter);
        }
    }

    wrap = new JSFilterList(filter_list);
    wrap->Wrap(info.This());

    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(JSFilterList::FilterString)
{
    Nan::HandleScope scope;
    static const unsigned int DEFAULT_LENGTH_LIMIT = 1000;

    std::string input = *Nan::Utf8String(info[0]);
    bool filter_links = Nan::To<bool>(info[1]).FromMaybe(false);
    uint32_t length_limit = DEFAULT_LENGTH_LIMIT;
    if (info[2]->IsNumber())
    {
        length_limit = Nan::To<uint32_t>(info[2]).FromMaybe(length_limit);
    }

    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(info.This());

    wrap->m_FilterList.exec(&input, filter_links, length_limit);

    Local<String> rv;
    if (!Nan::New<String>(input).ToLocal(&rv))
    {
        Nan::ThrowError("Unable to create return value");
        return;
    }

    info.GetReturnValue().Set(rv);
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
        if (!Util::ToJSObject(*it, filter))
        {
            Nan::ThrowError("Unable to convert filter to JS object");
            return;
        }

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

    Local<Object> obj;
    if (!Nan::To<Object>(info[0]).ToLocal(&obj))
    {
        Nan::ThrowTypeError("Filter to be updated must be an object");
        return;
    }

    Local<Value> key;
    if (!Nan::New<String>("name").ToLocal(&key))
    {
        Nan::ThrowError("Unable to create object key for access");
        return;
    }

    Local<Value> nameVal;
    if (!Nan::Get(obj, key).ToLocal(&nameVal))
    {
        Nan::ThrowError("Unable to extract name from filter object");
        return;
    }

    std::string name = *Nan::Utf8String(nameVal);
    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(info.This());
    Filter *filter = wrap->m_FilterList.find_filter(name);

    if (filter == NULL)
    {
        Nan::ThrowError("Filter to be updated does not exist");
        return;
    }

    Local<Array> fields;
    if (!Nan::GetPropertyNames(obj).ToLocal(&fields))
    {
        Nan::ThrowError("Unable to get property names");
        return;
    }

    for (unsigned int i = 0; i < fields->Length(); i++)
    {
        Local<Value> fieldName;
        if (!Nan::Get(fields, i).ToLocal(&fieldName))
        {
            Nan::ThrowError("Unable to get field name");
            return;
        }

        std::string sfield = *Nan::Utf8String(fieldName);
        Local<Value> value;
        if (!Nan::Get(obj, fieldName).ToLocal(&value))
        {
            Nan::ThrowError("Unable to get field value");
            return;
        }

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
            filter->set_source(*Nan::Utf8String(value));
        }
        else if (sfield == "replace")
        {
            filter->set_replacement(*Nan::Utf8String(value));
        }
        else if (sfield == "flags")
        {
            filter->set_flags(*Nan::Utf8String(value));
        }
        else if (sfield == "active")
        {
            filter->set_active(Nan::To<bool>(value).FromMaybe(false));
        }
        else if (sfield == "filterlinks")
        {
            filter->set_filter_links(Nan::To<bool>(value).FromMaybe(false));
        }
    }

    Local<Object> retval = Nan::New<Object>();
    if (!Util::ToJSObject(*filter, retval))
    {
        Nan::ThrowError("Unable to pack filter to JS object");
        return;
    }

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

    Local<Object> obj;
    if (!Nan::To<Object>(info[0]).ToLocal(&obj))
    {
        Nan::ThrowTypeError("Unable to convert argument to object");
        return;
    }

    Local<Value> key;
    if (!Nan::New<String>("name").ToLocal(&key))
    {
        Nan::ThrowError("Unable to create object key for access");
        return;
    }

    Local<Value> nameVal;
    if (!Nan::Get(obj, key).ToLocal(&nameVal))
    {
        Nan::ThrowError("Unable to extract name from filter object");
        return;
    }

    std::string name = *Nan::Utf8String(nameVal);
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

    Nan::Maybe<int32_t> i32from = Nan::To<int32_t>(info[0]);
    Nan::Maybe<int32_t> i32to = Nan::To<int32_t>(info[1]);

    if (i32from.IsNothing() || i32to.IsNothing())
    {
        Nan::ThrowError("Cannot convert from/to positions to int32");
        return;
    }

    std::vector<Filter>::size_type from = i32from.FromJust(),
                                   to   = i32to.FromJust();

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

    Local<Object> f;
    if (!Nan::To<Object>(info[0]).ToLocal(&f))
    {
        Nan::ThrowTypeError("Unable to convert argument to object");
        return;
    }

    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(info.This());

    std::string name;
    if (!Util::SafeGetString(f, "name", name))
    {
        Nan::ThrowTypeError("Unable to get filter name as string");
        return;
    }

    Filter *filter = wrap->m_FilterList.find_filter(name);

    if (filter != NULL)
    {
        Nan::ThrowError(("Filter '" + name + "' already exists.  Please choose a different name").c_str());
        return;
    }

    Filter newFilter;
    if (!Util::FromJSObject(f, newFilter))
    {
        Nan::ThrowTypeError("Invalid filter");
        return;
    }

    wrap->m_FilterList.add_filter(newFilter);
}

NAN_PROPERTY_GETTER(JSFilterList::GetLength)
{
    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(info.This());

    info.GetReturnValue().Set(Nan::New<Number>(wrap->m_FilterList.size()));
}

NAN_METHOD(JSFilterList::QuoteMeta)
{
    Nan::HandleScope scope;

    std::string quoted = pcrecpp::RE::QuoteMeta(*Nan::Utf8String(info[0]));

    info.GetReturnValue().Set(Nan::New<String>(quoted).ToLocalChecked());
}

NAN_METHOD(JSFilterList::CheckValidRegex)
{
    Nan::HandleScope scope;

    pcrecpp::RE re(*Nan::Utf8String(info[0]), DEFAULT_FLAGS);
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

void Init(Local<Object> exports, Local<Object> module)
{
    JSFilterList::Init();
    Local<FunctionTemplate> constructor_handle = Nan::New(constructor);

    Nan::Set(module, Nan::New<String>("exports").ToLocalChecked(),
            Nan::GetFunction(constructor_handle).ToLocalChecked());
};

NODE_MODULE(cytubefilters, Init);
