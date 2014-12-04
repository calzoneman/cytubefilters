#pragma once

#include <pcrecpp.h>

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

        const std::string& source() const;
        const std::string& name() const;
        const std::string& replacement() const;
        std::string flags() const;
        bool exec(std::string* input) const;
        bool active() const;
        bool filter_links() const;

    private:
        pcrecpp::RE *m_RE;
        std::string m_Replacement;
        std::string m_Name;
        bool m_Global;
        bool m_Active;
        bool m_FilterLinks;
        int m_Options;
};
