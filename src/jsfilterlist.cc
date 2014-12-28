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
        return NanThrowError("updateFilter expects 1 argument");
    }

    if (!args[0]->IsObject())
    {
        return NanThrowTypeError("Filter to be updated must be an object");
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
        Local<Value> value = obj->Get(field);

        if (!value->IsString() && (sfield == "source" || sfield == "replace" ||
            sfield == "flags"))
        {
            return NanThrowTypeError(("Field " + sfield + " must be a string").c_str());
        }
        else if (!value->IsBoolean() && (sfield == "active" || sfield == "filterlinks"))
        {
            return NanThrowTypeError(("Field " + sfield + " must be a boolean").c_str());
        }

        if (sfield == "source")
        {
            filter->set_source(*String::Utf8Value(value->ToString()));
        }
        else if (sfield == "replace")
        {
            filter->set_replacement(*String::Utf8Value(value->ToString()));
        }
        else if (sfield == "flags")
        {
            filter->set_flags(*String::Utf8Value(value->ToString()));
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

    Local<Object> retval = NanNew<Object>();
    Util::PackFilter(*filter, retval);

    NanReturnValue(retval);
}

NAN_METHOD(JSFilterList::RemoveFilter)
{
    NanScope();

    if (args.Length() < 1)
    {
        return NanThrowError("removeFilter expects 1 argument");
    }

    if (!args[0]->IsObject())
    {
        return NanThrowTypeError("Filter to be removed must be an object");
    }

    Local<Object> obj = args[0]->ToObject();

    std::string name = *String::Utf8Value(obj->Get(Util::NameField)->ToString());
    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(args.This());

    NanReturnValue(NanNew<Boolean>(wrap->m_FilterList.remove_filter(name)));
}

NAN_METHOD(JSFilterList::MoveFilter)
{
    NanScope();

    if (args.Length() != 2)
    {
        return NanThrowError("moveFilter expects 2 arguments");
    }

    if (!args[0]->IsInt32() || !args[1]->IsInt32())
    {
        return NanThrowTypeError("Arguments 'from' and 'to' must both be integers");
    }

    std::vector<Filter>::size_type from = args[0]->Int32Value(),
                                   to = args[1]->Int32Value();

    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(args.This());
    if (from >= wrap->m_FilterList.size() || to >= wrap->m_FilterList.size())
    {
        return NanThrowError("Argument out of range");
    }

    wrap->m_FilterList.move_filter(from, to);
    NanReturnUndefined();
}

NAN_METHOD(JSFilterList::AddFilter)
{
    NanScope();

    if (args.Length() != 1)
    {
        return NanThrowError("addFilter expects 1 argument");
    }

    if (!args[0]->IsObject())
    {
        return NanThrowTypeError("Filter to add must be an object");
    }

    Local<Object> f = args[0]->ToObject();
    if (!Util::ValidFilter(f))
    {
        return NanThrowError("Invalid filter");
    }

    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(args.This());
    std::string name = *String::Utf8Value(f->Get(Util::NameField)->ToString());
    Filter *filter = wrap->m_FilterList.find_filter(name);

    if (filter != NULL)
    {
        return NanThrowError(("Filter '" + name + "' already exists.  Please choose a different name").c_str());
    }

    wrap->m_FilterList.add_filter(Util::NewFilter(f));
    NanReturnUndefined();
}

NAN_GETTER(JSFilterList::GetLength)
{
    NanScope();

    JSFilterList *wrap = ObjectWrap::Unwrap<JSFilterList>(args.This());

    NanReturnValue(NanNew<Number>(wrap->m_FilterList.size()));
}

NAN_METHOD(JSFilterList::QuoteMeta)
{
    NanScope();

    std::string quoted = pcrecpp::RE::QuoteMeta(*String::Utf8Value(args[0]->ToString()));

    NanReturnValue(NanNew<String>(quoted));
}

NAN_METHOD(JSFilterList::CheckValidRegex)
{
    NanScope();

    pcrecpp::RE re(*String::Utf8Value(args[0]->ToString()));
    if (re.error().size() > 0)
    {
        return NanThrowError(re.error().c_str());
    }

    NanReturnValue(NanNew<Boolean>(true));
}

void JSFilterList::Init()
{
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(JSFilterList::New);
    NanAssignPersistent(constructor, tpl);
    tpl->SetClassName(NanNew<String>("FilterList"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->Set(NanNew<String>("quoteMeta"),
        FunctionTemplate::New(JSFilterList::QuoteMeta));
    tpl->Set(NanNew<String>("checkValidRegex"),
        FunctionTemplate::New(JSFilterList::CheckValidRegex));

    tpl->InstanceTemplate()->Set(NanNew<String>("filter"),
        FunctionTemplate::New(JSFilterList::FilterString));
    tpl->InstanceTemplate()->Set(NanNew<String>("pack"),
        FunctionTemplate::New(JSFilterList::Pack));
    tpl->InstanceTemplate()->Set(NanNew<String>("addFilter"),
        FunctionTemplate::New(JSFilterList::AddFilter));
    tpl->InstanceTemplate()->Set(NanNew<String>("updateFilter"),
        FunctionTemplate::New(JSFilterList::UpdateFilter));
    tpl->InstanceTemplate()->Set(NanNew<String>("removeFilter"),
        FunctionTemplate::New(JSFilterList::RemoveFilter));
    tpl->InstanceTemplate()->Set(NanNew<String>("moveFilter"),
        FunctionTemplate::New(JSFilterList::MoveFilter));

    tpl->InstanceTemplate()->SetAccessor(NanNew<String>("length"),
        JSFilterList::GetLength);
}

void Init(Handle<Object> exports, Handle<Object> module)
{
    JSFilterList::Init();
    Util::Init();
    Local<FunctionTemplate> constructor_handle = NanNew(constructor);

    module->Set(NanNew<String>("exports"), constructor_handle->GetFunction());
}

NODE_MODULE(cytubefilters, Init);
