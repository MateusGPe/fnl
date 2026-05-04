#pragma once
#include <FL/Fl.H>

namespace mui
{
    class System
    {
    public:
        static void init_threads()
        {
            Fl::lock();
        }

        static void add_timeout(double t, Fl_Timeout_Handler cb, void *data = nullptr)
        {
            Fl::add_timeout(t, cb, data);
        }

        static void repeat_timeout(double t, Fl_Timeout_Handler cb, void *data = nullptr)
        {
            Fl::repeat_timeout(t, cb, data);
        }

        static void remove_timeout(Fl_Timeout_Handler cb, void *data = nullptr)
        {
            Fl::remove_timeout(cb, data);
        }

        static int run()
        {
            return Fl::run();
        }

        static int check()
        {
            return Fl::check();
        }

        static void flush()
        {
            Fl::flush();
        }

        static double version()
        {
            return Fl::version();
        }
    };
}
