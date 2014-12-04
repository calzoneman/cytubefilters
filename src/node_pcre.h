#ifndef _NODE_PCRE_H
#define _NODE_PCRE_H

#include <node.h>
#include <nan.h>
#include <pcrecpp.h>

class PCREWrap : public node::ObjectWrap {
    public:
        static void Init();

    private:
        explicit PCREWrap(std::string& regexp, const pcrecpp::RE_Options& options);
        ~PCREWrap();

        static NAN_METHOD(New);
        static NAN_METHOD(Match);
        static NAN_METHOD(Replace);
        static NAN_METHOD(ReplaceAll);

        static NAN_GETTER(GetSource);

        pcrecpp::RE *m_RE;
};

#endif
