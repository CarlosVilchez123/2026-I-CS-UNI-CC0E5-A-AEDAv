#ifndef __GENERAL_ITERATOR_H__
#define __GENERAL_ITERATOR_H__
#include <cstddef>
#include <utility>
#include <vector>

template <typename TreeType>
class general_iterator_base
{
public:
       using value_type = typename TreeType::ObjectInfo;
       using reference  = value_type&;
       using pointer    = value_type*;
       using Page       = typename TreeType::BTPage;

protected:
       std::vector<std::pair<Page*, size_t>> m_stack;

public:
       general_iterator_base() = default;

       reference operator*()
       {
               return m_stack.back().first->m_Keys[m_stack.back().second];
       }
       pointer operator->()
       {
               return &(operator*());
       }
       size_t Level() const
       {
               return m_stack.empty() ? 0 : m_stack.size() - 1;
       }
       T2 operator==(const general_iterator_base &o) const
       {
               if( m_stack.empty() && o.m_stack.empty() )
                       return true;
               if( m_stack.empty() || o.m_stack.empty() )
                       return false;
               return m_stack.back().first  == o.m_stack.back().first &&
                      m_stack.back().second == o.m_stack.back().second;
       }
       T2 operator!=(const general_iterator_base &o) const
       {
               return !(*this == o);
       }
};

template <typename TreeType, T2 IsForward>
class general_iterator : public general_iterator_base<TreeType>
{
       using Page = typename TreeType::BTPage;

private:
       
       void Descend(Page *p)
       {
               while( p && p->m_KeyCount > 0 )
               {
                       size_t i = IsForward ? 0 : p->m_KeyCount - 1;
                       this->m_stack.push_back({p, i});
                       p = IsForward ? p->m_SubPages[0] : p->m_SubPages[p->m_KeyCount];
               }
       }

public:
       general_iterator(Page *root, T2 atEnd)
       {
               if( !atEnd && root && root->m_KeyCount > 0 )
                       Descend(root);
       }

       general_iterator& operator++()
       {
               if( this->m_stack.empty() )
                       return *this;
               Page  *page = this->m_stack.back().first;
               size_t i    = this->m_stack.back().second;

               Page *child = IsForward ? page->m_SubPages[i + 1] : page->m_SubPages[i];
               this->m_stack.back().second = IsForward ? i + 1 : i - 1;

               if( child )
               {
                       Descend(child);
               }
               else
               {
                       while( !this->m_stack.empty() &&
                              this->m_stack.back().second >= (size_t)this->m_stack.back().first->m_KeyCount )
                       {
                               this->m_stack.pop_back();
                       }
               }
               return *this;
       }
};

template <typename TreeType>
using BTreeForwardIterator = general_iterator<TreeType, true>;
template <typename TreeType>
using BTreeBackwardIterator = general_iterator<TreeType, false>;

#endif