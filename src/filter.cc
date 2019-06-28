#include <pcrecpp.h>

#include "./filter.h"

#define MATCH_LIMIT 5000

Filter::Filter() : m_RE(NULL)
{
}

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
    this->set_flags(flags);
    pcrecpp::RE_Options options(this->m_Flags);
    options.set_match_limit(MATCH_LIMIT);

    this->m_RE = new pcrecpp::RE(source, options);
}

Filter::Filter(const Filter& copy) : m_Replacement(copy.m_Replacement),
    m_Name(copy.m_Name),
    m_Global(copy.m_Global),
    m_Active(copy.m_Active),
    m_FilterLinks(copy.m_FilterLinks)
{
    this->m_Flags = copy.m_Flags;
    pcrecpp::RE_Options options(this->m_Flags);
    options.set_match_limit(MATCH_LIMIT);
    this->m_RE = new pcrecpp::RE(copy.m_RE->pattern(), options);
}

Filter& Filter::operator=(const Filter& rhs)
{
    this->m_Replacement = rhs.m_Replacement;
    this->m_Name = rhs.m_Name;
    this->m_Global = rhs.m_Global;
    this->m_Active = rhs.m_Active;
    this->m_FilterLinks = rhs.m_FilterLinks;

    delete this->m_RE;
    this->m_Flags = rhs.m_Flags;
    pcrecpp::RE_Options options(this->m_Flags);
    options.set_match_limit(MATCH_LIMIT);
    this->m_RE = new pcrecpp::RE(rhs.m_RE->pattern(), options);
    return *this;
}

Filter::~Filter()
{
    if (this->m_RE != NULL) delete this->m_RE;
}

const std::string& Filter::name() const
{
    return this->m_Name;
}

const std::string& Filter::source() const
{
    return this->m_RE->pattern();
}

void Filter::set_source(const std::string& source)
{
    delete this->m_RE;
    pcrecpp::RE_Options options(this->m_Flags);
    options.set_match_limit(MATCH_LIMIT);
    this->m_RE = new pcrecpp::RE(source, options);
}

const std::string& Filter::replacement() const
{
    return this->m_Replacement;
}

void Filter::set_replacement(const std::string& replacement)
{
    this->m_Replacement = replacement;
}

std::string Filter::flags() const
{
    std::string flags = "";
    if (this->m_Global)                 flags.push_back('g');
    if (this->m_Flags & PCRE_CASELESS)  flags.push_back('i');
    if (this->m_Flags & PCRE_MULTILINE) flags.push_back('m');

    return flags;
}

void Filter::set_flags(const std::string& flags)
{
    this->m_Flags = DEFAULT_FLAGS;
    for (size_t i = 0; i < flags.size(); i++)
    {
        switch (flags[i])
        {
            case 'i':
                this->m_Flags |= PCRE_CASELESS;
                break;
            case 'g':
                this->m_Global = true;
                break;
            case 'm':
                this->m_Flags |= PCRE_MULTILINE;
                break;
        }
    }
}

bool Filter::active() const
{
    return this->m_Active;
}

void Filter::set_active(bool active)
{
    this->m_Active = active;
}

bool Filter::filter_links() const
{
    return this->m_FilterLinks;
}

void Filter::set_filter_links(bool filter_links)
{
    this->m_FilterLinks = filter_links;
}

bool Filter::exec(std::string* input, unsigned int length_limit) const
{
    if (this->m_Global)
    {
        return this->m_RE->GlobalReplace(this->m_Replacement, input, length_limit);
    }
    else
    {
        return this->m_RE->Replace(this->m_Replacement, input);
    }
}
