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

Filter* FilterList::find_filter(const std::string& name)
{
    std::vector<Filter>::iterator it;
    for (it = this->m_Filters.begin(); it < this->m_Filters.end(); it++)
    {
        if (it->name() == name)
        {
            return &(*it);
        }
    }

    return NULL;
}

bool FilterList::remove_filter(const std::string& name)
{
    std::vector<Filter>::iterator it;
    for (it = this->m_Filters.begin(); it < this->m_Filters.end(); it++)
    {
        if (it->name() == name)
        {
            this->m_Filters.erase(it);
            return true;
        }
    }

    return false;
}

void FilterList::move_filter(unsigned int from, unsigned int to)
{
    std::swap(this->m_Filters[from], this->m_Filters[to]);
}

void FilterList::exec(std::string* input, bool filter_links, unsigned int length_limit)
{
    std::vector<Filter>::iterator it;
    for (it = this->m_Filters.begin(); it < this->m_Filters.end(); it++)
    {
        if (!it->active() || (filter_links && !it->filter_links()))
            continue;

        it->exec(input, length_limit);
    }
}

const std::vector<Filter>& FilterList::filters() const
{
    return this->m_Filters;
}

std::vector<Filter>::size_type FilterList::size() const
{
    return this->m_Filters.size();
}
