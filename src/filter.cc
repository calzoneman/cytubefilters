#include <pcrecpp.h>

#include "./filter.h"

#define MATCH_LIMIT 5000

Filter::Filter(const std::string& name,
    const std::string& source,
    const std::string& flags,
    const std::string& replacement,
    bool active,
    bool filter_links)
    : m_Replacement(replacement),
    m_Name(name),
    m_Global(false),
    m_Active(active),
    m_FilterLinks(filter_links)
{
    pcrecpp::RE_Options options;
    options.set_utf8(true);
    options.set_match_limit(MATCH_LIMIT);

    for (size_t i = 0; i < flags.size(); i++)
    {
        switch (flags[i])
        {
            case 'i':
                options.set_caseless(true);
                break;
            case 'g':
                this->m_Global = true;
                break;
            case 'm':
                options.set_multiline(true);
                break;
        }
    }

    this->m_RE = new pcrecpp::RE(source, options);
    this->m_Options = options.all_options();
}

Filter::Filter(const Filter& copy) : m_Replacement(copy.m_Replacement),
    m_Name(copy.m_Name),
    m_Global(copy.m_Global),
    m_Active(copy.m_Active),
    m_FilterLinks(copy.m_FilterLinks)
{
    pcrecpp::RE_Options options(copy.m_Options);
    this->m_RE = new pcrecpp::RE(copy.m_RE->pattern(), options);
    this->m_Options = options.all_options();
}

Filter& Filter::operator=(const Filter& rhs)
{
    this->m_Replacement = rhs.m_Replacement;
    this->m_Name = rhs.m_Name;
    this->m_Global = rhs.m_Global;
    this->m_Active = rhs.m_Active;
    this->m_FilterLinks = rhs.m_FilterLinks;

    delete this->m_RE;
    pcrecpp::RE_Options options(rhs.m_Options);
    this->m_RE = new pcrecpp::RE(rhs.m_RE->pattern(), options);
    this->m_Options = options.all_options();
    return *this;
}

Filter::~Filter()
{
    delete this->m_RE;
}

const std::string& Filter::source() const
{
    return this->m_RE->pattern();
}

const std::string& Filter::name() const
{
    return this->m_Name;
}

const std::string& Filter::replacement() const
{
    return this->m_Replacement;
}

std::string Filter::flags() const
{
    std::string flags = "";
    pcrecpp::RE_Options copy(this->m_Options);
    if (this->m_Global)   flags.push_back('g');
    if (copy.caseless())  flags.push_back('i');
    if (copy.multiline()) flags.push_back('m');

    return flags;
}

bool Filter::exec(std::string* input) const
{
    if (this->m_Global)
    {
        return this->m_RE->GlobalReplace(this->m_Replacement, input);
    }
    else
    {
        return this->m_RE->Replace(this->m_Replacement, input);
    }
}

bool Filter::active() const
{
    return this->m_Active;
}

bool Filter::filter_links() const
{
    return this->m_FilterLinks;
}
