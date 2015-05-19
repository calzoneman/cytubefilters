#pragma once

#include <pcrecpp.h>

#define DEFAULT_FLAGS PCRE_UTF8 | PCRE_JAVASCRIPT_COMPAT

class Filter
{
    public:
        Filter(const std::string& name,
            const std::string& source,
            const std::string& flags,
            const std::string& replacement,
            bool active,
            bool filter_links);
        Filter(const Filter& copy);
        ~Filter();

        Filter& operator=(const Filter& rhs);

        const std::string& name() const;

        const std::string& source() const;
        void set_source(const std::string& source);

        const std::string& replacement() const;
        void set_replacement(const std::string& replacement);

        std::string flags() const;
        void set_flags(const std::string& flags);

        bool active() const;
        void set_active(bool active);

        bool filter_links() const;
        void set_filter_links(bool filter_links);

        bool exec(std::string* input) const;

    private:
        pcrecpp::RE *m_RE;
        std::string m_Replacement;
        std::string m_Name;
        bool m_Global;
        bool m_Active;
        bool m_FilterLinks;
        int m_Flags;
};
