#ifndef nfw_shared_ptr_h
#define nfw_shared_ptr_h

namespace nfw {

template<class T>
class shared_ptr
{
public:
    /// @brief Default constructor
    shared_ptr(void) throw() : // nothrow
        px(NULL),
        pn(NULL)
    {
    }
    /// @brief Constructor with the provided pointer to manage
    explicit shared_ptr(T* p) : // throw std::bad_alloc
        px(p),
        pn(NULL)
    {
        acquire();
    }
    /// @brief Copy constructor (used by the copy-and-swap idiom)
    shared_ptr(const shared_ptr& ptr) : // throw std::bad_alloc
        px(ptr.px),
        pn(ptr.pn)
    {
        acquire();
    }
    /// @brief Assignment operator using the copy-and-swap idiom (copy constructor and swap method)
    shared_ptr& operator=(shared_ptr ptr) throw() // nothrow
    {
        swap(ptr);
        return *this;
    }
    /// @brief the destructor release its ownership
    inline ~shared_ptr(void) throw() // nothrow
    {
        release();
    }
    /// @brief this reset release its ownership
    inline void reset(void) throw() // nothrow
    {
        release();
    }
    /// @brief this reset release its ownership and re-acquire another one
    void reset(T* p) throw() // nothrow
    {
        release();
        px = p;
        pn = NULL;
        acquire();
    }

    /// @brief Swap method for the copy-and-swap idiom (copy constructor and swap method)
    void swap(shared_ptr& lhs) throw() // nothrow
    {
        std::swap(px, lhs.px);
        std::swap(pn, lhs.pn);
    }

    // reference counter operations :
    inline operator bool() const throw() // nothrow
    {
        return (0 < use_count());
    }
    inline bool unique(void)  const throw() // nothrow
    {
        return (1 == use_count());
    }
    long use_count(void)  const throw() // nothrow
    {
        long count = 0;
        if (NULL != pn)
        {
            count = *pn;
        }
        return count;
    }

    // underlying pointer operations :
    inline T& operator*()  const throw() // nothrow
    {
        return *px;
    }
    inline T* operator->() const throw() // nothrow
    {
        return px;
    }
    inline T* get(void)  const throw() // nothrow
    {
        return px;
    }

    // comparaison operators
    inline bool operator== (const shared_ptr& ptr) const
    {
        return (px == ptr.px);
    }
    inline bool operator== (const T* p) const
    {
        return (px == p);
    }
    inline bool operator!= (const shared_ptr& ptr) const
    {
        return (px != ptr.px);
    }
    inline bool operator!= (const T* p) const
    {
        return (px != p);
    }
    inline bool operator<= (const shared_ptr& ptr) const
    {
        return (px <= ptr.px);
    }
    inline bool operator<= (const T* p) const
    {
        return (px <= p);
    }
    inline bool operator< (const shared_ptr& ptr) const
    {
        return (px < ptr.px);
    }
    inline bool operator< (const T* p) const
    {
        return (px < p);
    }
    inline bool operator>= (const shared_ptr& ptr) const
    {
        return (px >= ptr.px);
    }
    inline bool operator>= (const T* p) const
    {
        return (px >= p);
    }
    inline bool operator> (const shared_ptr& ptr) const
    {
        return (px > ptr.px);
    }
    inline bool operator> (const T* p) const
    {
        return (px > p);
    }

private:
    /// @brief acquire/share the ownership of the px pointer, initializing the reference counter
    void acquire(void)
    {
        if (NULL != px)
        {
            if (NULL == pn)
            {
                try
                {
                    pn = new long(1);
                }
                catch (std::bad_alloc&)
                {
                    delete px;
                    throw;
                }
            }
            else
            {
                ++(*pn);
            }
        }
    }

    /// @brief release the ownership of the px pointer, destroying the object when appropriate
    void release(void) throw() // nothrow
    {
        if (NULL != pn)
        {
            --(*pn);
            if (0 == *pn)
            {
                delete px;
                px = NULL;
                delete pn;
                pn = NULL;
            }
        }
    }

private:
    T*      px; //!< Native pointer
    long*   pn; //!< Reference counter
};

}

#endif
