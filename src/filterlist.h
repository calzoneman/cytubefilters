#pragma once

#include <vector>

#include "./filter.h"

class FilterList
{
    public:
        FilterList();
        ~FilterList();

        void add_filter(const Filter& filter);
        Filter* find_filter(const std::string& name);
        bool remove_filter(const std::string& name);
        void move_filter(unsigned int from, unsigned int to);

        void exec(std::string* input, bool filter_links);
        const std::vector<Filter>& filters() const;
        std::vector<Filter>::size_type size() const;
    private:
        std::vector<Filter> m_Filters;
};
