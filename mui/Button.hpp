#pragma once
#include "Policies.hpp"
#include <FL/Fl_Button.H>
#include <utility>

namespace mui
{
    using namespace policy;
    // 1. Stack the behavioral backend FLTK policies
    using Button = StandardButtonDraw<HoverTracker<AutoThemed<CallbackRouter<Fl_Button>>>>;
} // namespace mui