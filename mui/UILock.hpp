#pragma once
#include <FL/Fl.H>

namespace mui
{
    class UILock
    {
    public:
        UILock()
        {
            Fl::lock();
        }

        ~UILock()
        {
            Fl::unlock();
            Fl::awake();
        }

        UILock(const UILock &) = delete;
        UILock &operator=(const UILock &) = delete;
    };
}
