#include <node.h>
#include <nan.h>
#include <pcrecpp.h>
#include "./node_pcre.h"

using v8::String;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Object;
using v8::Persistent;
using v8::Local;
using v8::Array;

static Persistent<FunctionTemplate> constructor;

PCREWrap::PCREWrap(std::string& regexp, const pcrecpp::RE_Options& options)
{
    this->m_RE = new pcrecpp::RE(regexp, options);
}

PCREWrap::~PCREWrap()
{
    delete this->m_RE;
}

/*
 * PCRE constructor
 */
NAN_METHOD(PCREWrap::New)
{
    NanScope();
    if (args.Length() == 0)
    {
        return NanThrowError("Missing parameter regexp");
    }

    if (!args[0]->IsString())
    {
        return NanThrowTypeError("Parameter regexp must be a string");
    }

    /* TODO allow JS to pass in options */
    pcrecpp::RE_Options options;
    options.set_utf8(true);
    /* Don't log errors to the console from C++-land,
     * instead it will be thrown as a JS Error below
     */
    std::string regexp(*String::Utf8Value(args[0]->ToString()));
    PCREWrap *wrap = new PCREWrap(regexp, options);
    wrap->Wrap(args.This());

    std::string error = wrap->m_RE->error();
    if (error.size() > 0)
    {
        return NanThrowError(NanNew<String>(error));
    }

    NanReturnThis();
}

/*
 * PCRE.match()
 */
NAN_METHOD(PCREWrap::Match)
{
    NanScope();

    PCREWrap *wrap = ObjectWrap::Unwrap<PCREWrap>(args.This());

    int num_groups = wrap->m_RE->NumberOfCapturingGroups();
    int i = 0;
    std::string group;
    pcrecpp::StringPiece input(*String::Utf8Value(args[0]->ToString()));

    Local<Array> match = NanNew<Array>();
    while (wrap->m_RE->FindAndConsume(&input, &group))
    {
        match->Set(i, NanNew<String>(group));
    }

    if (i == 0)
    {
        NanReturnNull();
    }

    NanReturnValue(match);
}

/*
 * PCRE.replace()
 */
NAN_METHOD(PCREWrap::Replace)
{
    NanScope();
    if (args.Length() != 2)
    {
        return NanThrowError("replace() expects 2 parameters (string, replacement)");
    }

    if (!args[0]->IsString() || !args[1]->IsString())
    {
        return NanThrowTypeError("Both parameters must be strings");
    }

    PCREWrap *wrap = ObjectWrap::Unwrap<PCREWrap>(args.This());
    std::string input(*String::Utf8Value(args[0]->ToString()));

    wrap->m_RE->Replace(*String::Utf8Value(args[1]->ToString()), &input);

    NanReturnValue(NanNew<String>(input));
}

/*
 * PCRE.replaceAll()
 */
NAN_METHOD(PCREWrap::ReplaceAll)
{
    NanScope();
    if (args.Length() != 2)
    {
        return NanThrowError("replaceAll() expects 2 parameters (string, replacement)");
    }

    if (!args[0]->IsString() || !args[1]->IsString())
    {
        return NanThrowTypeError("Both parameters must be strings");
    }

    PCREWrap *wrap = ObjectWrap::Unwrap<PCREWrap>(args.This());
    std::string input(*String::Utf8Value(args[0]->ToString()));

    wrap->m_RE->GlobalReplace(*String::Utf8Value(args[1]->ToString()), &input);

    NanReturnValue(NanNew<String>(input));
}

/*
 * Property PCRE.source
 */
NAN_GETTER(PCREWrap::GetSource)
{
    NanScope();
    PCREWrap *wrap = ObjectWrap::Unwrap<PCREWrap>(args.This());
    NanReturnValue(NanNew<String>(wrap->m_RE->pattern()));
}

/*
 * Class initializer.
 * This stuff is not done directly in the main Init() below
 * because it references private methods of the PCREWrap class
 */
void PCREWrap::Init()
{
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(PCREWrap::New);
    NanAssignPersistent(constructor, tpl);
    tpl->SetClassName(NanNew<String>("PCRE"));
    tpl->InstanceTemplate()->SetInternalFieldCount(2); // m_RE, m_Source

    /* instance methods */
    tpl->InstanceTemplate()->Set(NanNew<String>("match"),
        FunctionTemplate::New(PCREWrap::Match)->GetFunction());
    tpl->InstanceTemplate()->Set(NanNew<String>("replace"),
        FunctionTemplate::New(PCREWrap::Replace)->GetFunction());
    tpl->InstanceTemplate()->Set(NanNew<String>("replaceAll"),
        FunctionTemplate::New(PCREWrap::ReplaceAll)->GetFunction());

    /* instance properties */
    tpl->InstanceTemplate()->SetAccessor(NanNew<String>("source"), PCREWrap::GetSource);
}

void Init(Handle<Object> exports, Handle<Object> module)
{
    PCREWrap::Init();
    Local<FunctionTemplate> constructorHandle = NanNew(constructor);

    module->Set(NanNew<String>("exports"), constructorHandle->GetFunction());
}

NODE_MODULE(pcre, Init);
