#pragma once
#include <FL/Fl.H>

namespace mui
{
    namespace detail
    {
        template <typename T, void (T::*Method)()>
        void timeout_thunk(void *data)
        {
            T *obj = static_cast<T *>(data);
            (obj->*Method)();
        }

        template <typename T, void (T::*Method)()>
        void idle_thunk(void *data)
        {
            T *obj = static_cast<T *>(data);
            (obj->*Method)();
        }
    }

    class Dispatcher
    {
    public:
        template <typename T, void (T::*Method)()>
        static void awake(T *instance)
        {
            auto thunk = [](void *data)
            {
                T *obj = static_cast<T *>(data);
                (obj->*Method)();
            };
            Fl::awake(thunk, instance);
        }
        template <typename T, void (T::*Method)()>
        static void timeout(double t, T *instance)
        {
            Fl::add_timeout(t, &detail::timeout_thunk<T, Method>, instance);
        }
        template <typename T, void (T::*Method)()>
        static void repeat_timeout(double t, T *instance)
        {
            Fl::repeat_timeout(t, &detail::timeout_thunk<T, Method>, instance);
        }
        template <typename T, void (T::*Method)()>
        static void remove_timeout(T *instance)
        {
            Fl::remove_timeout(&detail::timeout_thunk<T, Method>, instance);
        }
        template <typename T, void (T::*Method)()>
        static void add_idle(T *instance)
        {
            Fl::add_idle(&detail::idle_thunk<T, Method>, instance);
        }
        template <typename T, void (T::*Method)()>
        static void remove_idle(T *instance)
        {
            Fl::remove_idle(&detail::idle_thunk<T, Method>, instance);
        }
    };
}
