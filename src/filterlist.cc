#include <vector>

#include "./filterlist.h"
#include "./filter.h"

FilterList::FilterList()
{
}

FilterList::~FilterList()
{
}

void FilterList::add_filter(const Filter& filter)
{
    this->m_Filters.push_back(filter);
}

void FilterList::update_filter(const Filter& filter)
{
    std::vector<Filter>::iterator it;
    for (it = this->m_Filters.begin(); it < this->m_Filters.end(); it++)
    {
        if (it->name() == filter.name())
        {
            *it = filter;
            break;
        }
    }
}

void FilterList::remove_filter(const Filter& filter)
{
    std::vector<Filter>::iterator it;
    for (it = this->m_Filters.begin(); it < this->m_Filters.end(); it++)
    {
        if (it->name() == filter.name())
        {
            this->m_Filters.erase(it);
            break;
        }
    }
}

void FilterList::move_filter(unsigned int from, unsigned int to)
{
    std::swap(this->m_Filters[from], this->m_Filters[to]);
}

void FilterList::exec(std::string* input, bool filter_links)
{
    std::vector<Filter>::iterator it;
    for (it = this->m_Filters.begin(); it < this->m_Filters.end(); it++)
    {
        if (!it->active() || (filter_links && !it->filter_links()))
            continue;

        it->exec(input);
    }
}

const std::vector<Filter>& FilterList::filters() const
{
    return this->m_Filters;
}
