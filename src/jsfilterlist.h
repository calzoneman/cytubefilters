#pragma once

#include <node.h>
#include <nan.h>

#include "./filterlist.h"

using node::ObjectWrap;
using v8::Array;

class JSFilterList : public ObjectWrap
{
    public:
        static void Init();

    private:
        explicit JSFilterList(const FilterList& filter_list);
        ~JSFilterList();

        static NAN_METHOD(New);
        static NAN_METHOD(FilterString);
        static NAN_METHOD(Pack);
        static NAN_METHOD(AddFilter);
        static NAN_METHOD(UpdateFilter);
        static NAN_METHOD(RemoveFilter);
        static NAN_METHOD(MoveFilter);

        static NAN_GETTER(GetLength);

        static NAN_METHOD(QuoteMeta);
        static NAN_METHOD(CheckValidRegex);

        FilterList m_FilterList;
};
