#ifndef SMART_POINTER_H
#define SMART_POINTER_H

#include "Utils.h"

template <class T>
class Ptr
{
private:
    T* data;  // pointer
    int* ref; // reference count
    void *cs; // critical section

public:
    Ptr()
    {
        data = 0;
        ref = new int;
        cs = Utils::AllocCriticalSection();
        *ref = 1;
    }

    Ptr(T* value)
    {
        data = value;
        ref = new int;
        cs = Utils::AllocCriticalSection();
        *ref = 1;
    }

    Ptr(const Ptr<T> &src)
    {
        Utils::Lock(src.cs);
        data = src.data;
        ref = src.ref;
        cs = src.cs;
        *ref += 1;
        Utils::Unlock(src.cs);
    }

    ~Ptr()
    {
        Utils::Lock(cs);
        *ref -= 1;
        Utils::Unlock(cs);

        if(*ref == 0)
        {
            delete ref;
            if (data != 0) delete data;
            Utils::DeleteCriticalSection(cs);
        }
    }

    T& operator*()
    {
        return *data;
    }

    T* operator->()
    {
        return data;
    }

    Ptr<T>& operator=(const Ptr<T>& src)
    {
        if (this != &src) // avoid self assignment
        {
            // Decrement the old reference count
            // if reference become zero delete the old data
            Utils::Lock(cs);
            *ref -= 1;
            Utils::Unlock(cs);

            if(*ref == 0)
            {
                delete ref;
                if (data != 0) delete data;
                Utils::DeleteCriticalSection(cs);
            }
            
            // Copy the data and reference pointer
            // and increment the reference count
            Utils::Lock(src.cs);
            data = src.data;
            ref = src.ref;
            cs = src.cs;
            *ref += 1;
            Utils::Unlock(src.cs);
        }
        return *this;
    }

    bool operator==(const Ptr<T> &right)
    {
        return data == right.data;
    }

    bool operator!=(const Ptr<T> &right)
    {
        return data != right.data;
    }
};

#endif
