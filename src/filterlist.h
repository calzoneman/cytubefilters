#pragma once

#include <vector>

#include "./filter.h"

class FilterList
{
    public:
        FilterList();
        ~FilterList();

        void add_filter(const Filter& filter);
        void update_filter(const Filter& filter);
        void remove_filter(const Filter& filter);
        void move_filter(unsigned int from, unsigned int to);

        void exec(std::string* input, bool filter_links);
        const std::vector<Filter>& filters() const;
    private:
        std::vector<Filter> m_Filters;
};
