#pragma once
#include <FL/Fl.H>

namespace mui
{
    namespace detail
    {
        template <typename T, void (T::*Method)()>
        struct Thunk
        {
            static void run(void *data)
            {
                if (data)
                {
                    (static_cast<T *>(data)->*Method)();
                }
            }
        };
    }

    class Dispatcher
    {
    public:
        template <typename T, void (T::*Method)()>
        static void awake(T *instance)
        {
            Fl::awake(&detail::Thunk<T, Method>::run, instance);
        }
        template <typename T, void (T::*Method)()>
        static void timeout(double t, T *instance)
        {
            Fl::add_timeout(t, &detail::Thunk<T, Method>::run, instance);
        }
        template <typename T, void (T::*Method)()>
        static void repeat_timeout(double t, T *instance)
        {
            Fl::repeat_timeout(t, &detail::Thunk<T, Method>::run, instance);
        }
        template <typename T, void (T::*Method)()>
        static void remove_timeout(T *instance)
        {
            Fl::remove_timeout(&detail::Thunk<T, Method>::run, instance);
        }
        template <typename T, void (T::*Method)()>
        static void add_idle(T *instance)
        {
            Fl::add_idle(&detail::Thunk<T, Method>::run, instance);
        }
        template <typename T, void (T::*Method)()>
        static void remove_idle(T *instance)
        {
            Fl::remove_idle(&detail::Thunk<T, Method>::run, instance);
        }
    };
}
